#include "findUvOverlaps.hpp"
#include "uvPoint.hpp"
#include "uvUtils.hpp"
#include "uvEdge.hpp"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

FindUvOverlaps::FindUvOverlaps()
{
}

FindUvOverlaps::~FindUvOverlaps()
{
}

MSyntax FindUvOverlaps::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-set", "-uvSet", MSyntax::kString);
    return syntax;
}

MStatus FindUvOverlaps::doIt(const MArgList& args)
{
    MStatus status;

    MArgDatabase argData(syntax(), args);

    status = argData.getCommandArgument(0, mSel);
    if (status != MS::kSuccess) {
        // if not specified, use current selections
        MGlobal::getActiveSelectionList(mSel);
    }

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);
    else
        verbose = false;

    if (argData.isFlagSet("-uvSet"))
        argData.getFlagArgument("-uvSet", 0, uvSet);
    else
        uvSet = "None";

    return redoIt();
}

MStatus FindUvOverlaps::redoIt()
{
    MStatus status;

    // INITILIZATION PROCESS
    //
    timer.beginTimer();
    int numSelection = mSel.length();
    int objectId = 1;
    for (int i = 0; i < numSelection; i++) {
        mSel.getDagPath(i, dagPath);

        // Check if specified object is geometry or not
        status = dagPath.extendToShape();
        if (status != MS::kSuccess) {
            if (verbose)
                MGlobal::displayInfo("Failed to extend to shape node.");
            continue;
        }

        if (dagPath.apiType() != MFn::kMesh) {
            if (verbose)
                MGlobal::displayInfo("Selected node : " + dagPath.fullPathName() + " is not mesh.");
            continue;
        }

        status = initializeObject(dagPath, objectId);
        if (status != MS::kSuccess) {
            MGlobal::displayWarning("Initialization failed at :" + dagPath.fullPathName());
            MGlobal::displayWarning(dagPath.fullPathName() + " will not be evaluated.");
            continue;
        }
        objectId++;
    }

    timer.endTimer();
    if (verbose) {
        UvUtils::displayTime("Initialization completed", timer.elapsedTime());
    }
    timer.clear();

    if (uvShellArrayMaster.size() == 0) {
        MGlobal::displayWarning("No meshes are found or selected.");
        return MS::kSuccess;
    }

    // CHECKING PROCESS
    //
    timer.beginTimer();
    if (uvShellArrayMaster.size() == 1) {
        // if there is only one uv shell, just send it to checker command.
        // don't need to check uv bounding box overlaps check.
        status = check(uvShellArrayMaster[0].edgeSet, 0);
        if (status != MS::kSuccess) {
            MGlobal::displayInfo("Error found in shell");
        }
    } else {
        // If there multiple uv shells, do BBox overlap check first, then if they overlaps,
        // make one combined shell and send it to checker command

        // Countainer for both overlapped shells and indivisual shells for checker
        std::vector<std::set<UvEdge>> shellArray;

        // Array like [0, 1, 3, 4 ...... nbUvShells]
        // UV shells with those indices will be independent shells that don't overlap
        // with any other shells in bouding box check.
        std::set<int> shellIndices;
        for (unsigned int i = 0; i < uvShellArrayMaster.size(); i++) {
            shellIndices.insert(i);
        }

        // Get combinations of shell indices eg. (0, 1), (0, 2), (1, 2),,,
        std::vector<std::vector<int>> shellCombinations;
        UvUtils::makeCombinations(uvShellArrayMaster.size(), shellCombinations);

        for (size_t i = 0; i < shellCombinations.size(); i++) {
            UvShell& shellA = uvShellArrayMaster[shellCombinations[i][0]];
            UvShell& shellB = uvShellArrayMaster[shellCombinations[i][1]];

            // Check if two bounding boxes of two UV shells are overlapped
            bool isOverlapped = UvUtils::isBoundingBoxOverlapped(
                shellA.uMin,
                shellA.uMax,
                shellA.vMin,
                shellA.vMax,
                shellB.uMin,
                shellB.uMax,
                shellB.vMin,
                shellB.vMax);

            if (isOverlapped) {
                // Check boundingbox check for two shells
                // If those two shells are overlapped, combine them into one single shell
                // and add to shellArray
                std::set<UvEdge> combinedEdges;
                combinedEdges.insert(shellA.edgeSet.begin(), shellA.edgeSet.end());
                combinedEdges.insert(shellB.edgeSet.begin(), shellB.edgeSet.end());
                shellArray.push_back(combinedEdges);

                // Remove from shellIndices as these shells don't have to be checked
                // as indivisula shells
                shellIndices.erase(shellA.shellIndex);
                shellIndices.erase(shellB.shellIndex);
            }
        }

        // Extract single shells and re-insert them to shellArray to be checked
        std::set<int>::iterator shIter;
        for (shIter = shellIndices.begin(); shIter != shellIndices.end(); ++shIter) {
            int index = *shIter;
            std::set<UvEdge>& edges = uvShellArrayMaster[index].edgeSet;
            shellArray.push_back(edges);
        }

        // Check each shell in multithreads
        //
        int threadCount = int(shellArray.size());

        std::thread* threadArray = new std::thread[threadCount];

        for (int i = 0; i < threadCount; i++) {
            threadArray[i] = std::thread(&FindUvOverlaps::check, this, std::ref(shellArray[i]), i);
        }

        for (int i = 0; i < threadCount; i++) {
            threadArray[i].join();
        }

        delete[] threadArray;
    }

    // Re-insert all results from temp vectors in each thread to Maya's MStringArray
    // for setResult command
    for (size_t i=0; i<resultVector.size(); i++) {
        resultStringArray.append(resultVector[i].c_str());
    }

    timer.endTimer();
    if (verbose) {
        UvUtils::displayTime("Check completed", timer.elapsedTime());
    }
    timer.clear();

    // Return final result
    MPxCommand::setResult(resultStringArray);

    return MS::kSuccess;
}

MStatus FindUvOverlaps::initializeObject(const MDagPath& dagPath, const int objectId)
{

    MStatus status;

    MFnMesh fnMesh(dagPath);
    numEdges = fnMesh.numEdges();

    // Temporary container to store uvShell objects for CURRENT OBJECT
    // These uvShell objects will be copied into uvShellArrayMaster at the end
    std::vector<UvShell> uvShellArrayTemp;

    // getUvShellsIds function gives wrong number of uv shells when accessing
    // to non-current uvSets. So just temporarily switch to target uvSet then switch
    // back to current uvSet at the end of function.
    MString currentUvSet = fnMesh.currentUVSetName();
    MString workUvSet;

    // Check if specified uv set exists
    MStringArray uvSetNames;
    bool uvSetFound = false;
    fnMesh.getUVSetNames(uvSetNames);
    for (unsigned int uv = 0; uv < uvSetNames.length(); uv++) {
        MString& uvSetName = uvSetNames[uv];
        if (uvSetName == uvSet) {
            uvSetFound = true;
            break;
        }
    }

    if (uvSet == "None") {
        workUvSet = fnMesh.currentUVSetName();
    } else if (uvSetFound == false) {
        return MS::kFailure;
    } else {
        fnMesh.setCurrentUVSetName(uvSet);
        workUvSet = uvSet;
    }
    const MString* uvSetPtr = &workUvSet;

    MIntArray uvShellIds;
    unsigned int nbUvShells;
    MFloatArray uArray;
    MFloatArray vArray;

    fnMesh.getUvShellsIds(uvShellIds, nbUvShells, uvSetPtr);
    fnMesh.getUVs(uArray, vArray, uvSetPtr);

    // if no UVs are detected on this mesh
    if (nbUvShells == 0) {
        MGlobal::displayError("No UVs are found.");
        return MS::kFailure;
    }

    // Setup uv shell objects
    uvShellArrayTemp.resize(nbUvShells);
    for (unsigned int i = 0; i < nbUvShells; i++) {
        UvShell shell(i);
        shell.uVector.reserve(uArray.length());
        shell.vVector.reserve(uArray.length());
        uvShellArrayTemp[i] = shell;
    }

    // Get UV values and add them to the shell
    int numUVs = fnMesh.numUVs(workUvSet);
    for (int uvId = 0; uvId < numUVs; uvId++) {
        float u = uArray[uvId];
        float v = vArray[uvId];
        UvShell& currentShell = uvShellArrayTemp[uvShellIds[uvId]];
        currentShell.uVector.emplace_back(u);
        currentShell.vVector.emplace_back(v);
    }

    // Setup bounding box information for each shell
    for (unsigned int id = 0; id < nbUvShells; id++) {
        UvShell& shell = uvShellArrayTemp[id];
        shell.uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
        shell.vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
        shell.uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
        shell.vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
    }

    MIntArray uvCounts;
    MIntArray uvIds;
    int uvCounter = 0;
    int nextCounter;
    fnMesh.getAssignedUVs(uvCounts, uvIds, uvSetPtr);
    int uvCountSize = uvCounts.length();
    
    std::vector<std::pair<int, int>> idPairVec;
    idPairVec.reserve(uvCountSize*4);
    
    for (unsigned int i = 0; i < uvCountSize; i++) {
        int numFaceUVs = uvCounts[i];
        for (int u=0; u<numFaceUVs; u++) {
            if (u == numFaceUVs-1) {
                nextCounter = uvCounter - numFaceUVs + 1;
            } else {
                nextCounter = uvCounter + 1;
            }
            
            int idA = uvIds[uvCounter];
            int idB = uvIds[nextCounter];
            
            std::pair<int, int> idPair;
            
            if (idA < idB) {
                idPair = std::make_pair(idA, idB);
            } else {
                idPair = std::make_pair(idB, idA);
            }
            
            idPairVec.emplace_back(idPair);
            uvCounter++;
        }
    }
    
    // Remove duplicate elements
    std::sort(idPairVec.begin(), idPairVec.end());
    idPairVec.erase(std::unique(idPairVec.begin(), idPairVec.end()), idPairVec.end());
    
    // Construct data struct for each thread
    objectData data;
    data.objectId = objectId;
    data.uvShellIds = &uvShellIds;
    data.uArray = &uArray;
    data.vArray = &vArray;
    data.idPairVecPtr = &idPairVec;
    data.path = dagPath.fullPathName().asChar();
    
    int numEdges = idPairVec.size();
    const int numThreads = 8;
    int threadRange = numEdges / numThreads;
    int divisionRemainder = numEdges % numThreads;
    
    int rangeBegin = 0;  // initial range start
    int rangeEnd = threadRange; // initial range end
    
    // Container for the results from each thread
    std::vector<std::vector<UvEdge>> threadEdgeVector;
    threadEdgeVector.resize(numThreads);
    for (int i=0; i<numThreads; i++) {
        threadEdgeVector[i].reserve(numEdges);
    }
    
    std::thread threadArray[numThreads];
    
    for (int i=0; i<numThreads; i++) {
        if (i == numThreads - 1) {
            // last thread
            rangeEnd += divisionRemainder;
        }
        
        data.begin = rangeBegin;
        data.end = rangeEnd;
        data.threadId = i;
        
        threadArray[i] = std::thread(&FindUvOverlaps::createUvEdge, this, data, std::ref(threadEdgeVector));
        
        rangeBegin += threadRange;
        rangeEnd += threadRange;
    }
    
    // thread join
    for (int i = 0; i < numThreads; i++) {
        threadArray[i].join();
    }
    
    // Extract edges from each thread results and insert them to shells
    std::vector<UvEdge>::iterator threadEdgeIter;
    for (int i=0; i<numThreads; i++) {
        std::vector<UvEdge>& threadEdges = threadEdgeVector[i];
        for (threadEdgeIter=threadEdges.begin(); threadEdgeIter != threadEdges.end(); ++threadEdgeIter) {
            UvEdge& e = *threadEdgeIter;
            uvShellArrayTemp[e.shellIndex].edgeSet.insert(e);
        }
    }
    
    // Copy uvShells in temp container to master container
    std::copy(uvShellArrayTemp.begin(), uvShellArrayTemp.end(), std::back_inserter(uvShellArrayMaster));

    return MS::kSuccess;
}

MStatus FindUvOverlaps::createUvEdge(objectData data, std::vector<std::vector<UvEdge>>& threadEdgeVector) {
    
    std::vector<std::pair<int, int>>& idPairArray = *data.idPairVecPtr;
    int objectId = data.objectId;
    int threadId = data.threadId;
    MIntArray& uvShellIds = *data.uvShellIds;
    std::string dagPathStr = data.path;
    MFloatArray& uArray = *data.uArray;
    MFloatArray& vArray = *data.vArray;
    
    
    std::string stringId;
    for (int i=data.begin; i<data.end; i++) {
        const int uvIdA = idPairArray[i].first;
        const int uvIdB = idPairArray[i].second;
        stringId = std::to_string((long long)objectId) + std::to_string((long long)uvIdA) + std::to_string((long long)uvIdB);
        int currentShellIndex = uvShellIds[uvIdA];
        
        std::string path_to_p1 = dagPathStr + ".map[" + std::to_string((long long)uvIdA) + "]";
        std::string path_to_p2 = dagPathStr + ".map[" + std::to_string((long long)uvIdB) + "]";
        
        UvPoint p1(uArray[uvIdA], vArray[uvIdA], uvIdA, currentShellIndex, path_to_p1);
        UvPoint p2(uArray[uvIdB], vArray[uvIdB], uvIdB, currentShellIndex, path_to_p2);
        
        UvEdge edge;
        if (p1 > p2) {
            edge.init(p2, p1, stringId, currentShellIndex);
        } else {
            edge.init(p1, p2, stringId, currentShellIndex);
        }
        
        threadEdgeVector[threadId].emplace_back(edge);
    }
    
    return MS::kSuccess;
}

MStatus FindUvOverlaps::check(const std::set<UvEdge>& edges, int threadNumber)
{
    // Container for all events. Items need to be always sorted.
    std::multiset<Event> eventQueue;

    // Create event objects from edge set
    int eventIndex = 0;
    for (std::set<UvEdge>::const_iterator iter = edges.begin(), end = edges.end(); iter != end; ++iter) {

        Event ev1(0, &(*iter), iter->begin, eventIndex);
        eventIndex += 1;
        Event ev2(1, &(*iter), iter->end, eventIndex);
        eventIndex += 1;

        eventQueue.insert(ev1);
        eventQueue.insert(ev2);
    }

    // Container for active edges to be tested intersections
    std::vector<UvEdge> statusQueue;
    statusQueue.reserve(edges.size());
    
    checkThreadData checkData;
    checkData.eventQueuePtr = &eventQueue;
    checkData.statusQueuePtr = &statusQueue;
    checkData.threadNumber = threadNumber;

    while (true) {
        if (eventQueue.empty()) {
            break;
        }
        Event firstEvent = *eventQueue.begin();
        eventQueue.erase(eventQueue.begin());
        
        checkData.currentEventPtr = &firstEvent;
        checkData.currentEdgePtr = firstEvent.edgePtr;
        checkData.otherEdgePtr = firstEvent.otherEdgePtr;
        checkData.sweepline = firstEvent.v;

        switch (firstEvent.eventType) {
        case Event::BEGIN:
            doBegin(checkData);
            break;
        case Event::END:
            doEnd(checkData);
            break;
        case Event::CROSS:
            doCross(checkData);
            break;
        default:
            if (verbose)
                MGlobal::displayError("Unknow exception");
            return MS::kFailure;
            break;
        }
    }
    return MS::kSuccess;
}

bool FindUvOverlaps::doBegin(checkThreadData& checkData)
{
    // Extract data
    const UvEdge& currentEdge = *checkData.currentEdgePtr;
    std::vector<UvEdge>& statusQueue = *checkData.statusQueuePtr;
    
    checkData.edgeA = &currentEdge;
    
    statusQueue.emplace_back(currentEdge);

    // if there are no edges to compare
    size_t numStatus = statusQueue.size();
    if (numStatus == 1) {
        return false;
    }

    // Update x values of intersection to the sweepline for all edges
    // in the statusQueue
    edgeUtils::setCrosingPoints(statusQueue, checkData.sweepline);
    std::sort(statusQueue.begin(), statusQueue.end());

    // StatusQueue was sorted so you have to find the edge added to the queue above and find its index
    std::vector<UvEdge>::iterator foundIter = std::find(statusQueue.begin(), statusQueue.end(), currentEdge);
    if (foundIter == statusQueue.end()) {
        // If the edge was not found in the queue, skip this function and go to next event
        return false;
    }

    if (foundIter == statusQueue.begin()) {
        // If first item, check the next edge
        checkData.edgeB = &*(++foundIter);
        checkEdgesAndCreateEvent(checkData);
        
    } else if (foundIter == statusQueue.end() - 1) {
        // if last iten in the statusQueue
        checkData.edgeB = &*(--foundIter);
        checkEdgesAndCreateEvent(checkData);
        
    } else {
        checkData.edgeB = &*(++foundIter);
        checkEdgesAndCreateEvent(checkData);
        
        checkData.edgeB = &*(foundIter-2);
        checkEdgesAndCreateEvent(checkData);
    }
    return true;
}

bool FindUvOverlaps::doEnd(checkThreadData& checkData)
{
    // Extract data
    const UvEdge& currentEdge = *checkData.currentEdgePtr;
    std::vector<UvEdge>& statusQueue = *checkData.statusQueuePtr;

    std::vector<UvEdge>::iterator foundIter = std::find(statusQueue.begin(), statusQueue.end(), currentEdge);
    if (foundIter == statusQueue.end()) {
        if (verbose)
            MGlobal::displayInfo("Failed to find the edge to be removed at the end event.");
        // if iter not found
        return false;
    }

    if (foundIter == statusQueue.begin() || foundIter == statusQueue.end()-1) {
        // if first or last item, do nothing
    } else {
        // check previous and next edge intersection as they can be next
        // each other after removing the current edge
        
        ++foundIter;
        checkData.edgeA = &*(foundIter); // next edge
        
        --foundIter;
        --foundIter;
        
        checkData.edgeB = &*(foundIter); // previous edge
        
        checkEdgesAndCreateEvent(checkData);
        
        ++foundIter; // Move back to original iterator position
    }

    // Remove current edge from the statusQueue
    statusQueue.erase(foundIter);
    return true;
}

bool FindUvOverlaps::doCross(checkThreadData& checkData)
{
    // Extract data
    std::vector<UvEdge>& statusQueue = *checkData.statusQueuePtr;
    
    if (statusQueue.size() <= 2) {
        return false;
    }

    const UvEdge& thisEdge = *checkData.currentEdgePtr;
    const UvEdge& otherEdge = *checkData.otherEdgePtr;
    std::vector<UvEdge>::iterator thisEdgeIter = std::find(statusQueue.begin(), statusQueue.end(), thisEdge);
    std::vector<UvEdge>::iterator otherEdgeIter = std::find(statusQueue.begin(), statusQueue.end(), otherEdge);
    
    if (thisEdgeIter == statusQueue.end() || otherEdgeIter == statusQueue.end()) {
        // if not found
        return false;
    }
    
    size_t thisIndex = std::distance(statusQueue.begin(), thisEdgeIter);
    size_t otherIndex = std::distance(statusQueue.begin(), otherEdgeIter);
    size_t small, big;

    if (thisIndex > otherIndex) {
        small = otherIndex;
        big = thisIndex;
    } else {
        small = thisIndex;
        big = otherIndex;
    }

    if (small == 0) {
        // Check the first edge and the one after next edge
        checkData.edgeA = &statusQueue[small];
        checkData.edgeB = &statusQueue[big + 1];
        checkEdgesAndCreateEvent(checkData);
        
    } else if (big == statusQueue.size() - 1) {
        // Check the last edge and the one before the previous edge
        checkData.edgeA = &statusQueue[small - 1];
        checkData.edgeB = &statusQueue[big];
        checkEdgesAndCreateEvent(checkData);
        
    } else {
        // Check the first edge and the one after next(third)
        checkData.edgeA = &statusQueue[small - 1];
        checkData.edgeB = &statusQueue[big];
        checkEdgesAndCreateEvent(checkData);
        
        // Check the second edge and the one after next(forth)
        checkData.edgeA = &statusQueue[small];
        checkData.edgeB = &statusQueue[big + 1];
        checkEdgesAndCreateEvent(checkData);
    }
    return false;
}

MStatus FindUvOverlaps::checkEdgesAndCreateEvent(checkThreadData& checkData)
{
    bool isParallel = false;
    const UvEdge& edgeA = *checkData.edgeA;
    const UvEdge& edgeB = *checkData.edgeB;
    std::multiset<Event>& eventQueue = *checkData.eventQueuePtr;
    
    if (edgeUtils::isEdgeIntersected(edgeA, edgeB, isParallel)) {

        float uv[2]; // countainer for uv inters point
        UvUtils::getEdgeIntersectionPoint(edgeA.begin.u,
                                          edgeA.begin.v,
                                          edgeA.end.u,
                                          edgeA.end.v,
                                          edgeB.begin.u,
                                          edgeB.begin.v,
                                          edgeB.end.u,
                                          edgeB.end.v,
                                          uv);

        resultVector.push_back(edgeA.begin.path);
        resultVector.push_back(edgeB.begin.path);
        resultVector.push_back(edgeA.end.path);
        resultVector.push_back(edgeB.end.path);

        if (isParallel == false) {
            Event crossEvent(2, uv[0], uv[1], &edgeA, &edgeB);
            eventQueue.insert(crossEvent);
        }
    }
    return MS::kSuccess;
}

void FindUvOverlaps::safeInsert(std::string &path) {
    std::lock_guard<std::mutex> locker(mtx);
    resultVector.push_back(path);
}

MStatus FindUvOverlaps::undoIt()
{
    return MS::kSuccess;
}

bool FindUvOverlaps::isUndoable() const
{
    return false;
}

void* FindUvOverlaps::creator()
{
    return new FindUvOverlaps;
}
