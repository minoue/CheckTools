#include "findUvOverlaps.h"
#include "uvPoint.h"
#include "uvUtils.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MTimer.h>
#include <maya/MFloatArray.h>

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
    syntax.addFlag("-mt", "-multiThread", MSyntax::kBoolean);
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

    if (argData.isFlagSet("-multiThread"))
        argData.getFlagArgument("-multiThread", 0, multiThread);
    else
        multiThread = false;

    return redoIt();
}

MStatus FindUvOverlaps::redoIt()
{
    MStatus status;

    MString timeStr;
    MTimer timer;

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
        timeStr.set(timer.elapsedTime());
        MGlobal::displayInfo("Initialization completed : " + timeStr + " seconds.");
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
        tempResultVector.resize(1);
        tempResultVector[0].reserve(1000);
        status = check(uvShellArrayMaster[0].unordered_edgeSet, 0);
        if (status != MS::kSuccess) {
            MGlobal::displayInfo("Error found in shell");
        }
    } else {
        // If there multiple uv shells, do BBox overlap check first, then if they overlaps,
        // make one combined shell and send it to checker command

        // Countainer for both overlapped shells and indivisual shells for checker
        std::vector<std::unordered_set<UvEdge, hash_edge>> shellArray;

        // Array like [0, 1, 3, 4 ...... nbUvShells]
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
                std::unordered_set<UvEdge, hash_edge> combinedEdges;
                combinedEdges.insert(shellA.unordered_edgeSet.begin(), shellA.unordered_edgeSet.end());
                combinedEdges.insert(shellB.unordered_edgeSet.begin(), shellB.unordered_edgeSet.end());
                shellArray.push_back(combinedEdges);

                // Remove from shellIndices as these shells don't have to be checked
                // as indivisula shells
                shellIndices.erase(shellA.shellIndex);
                shellIndices.erase(shellB.shellIndex);
            }
        }

        std::set<int>::iterator shIter;
        for (shIter = shellIndices.begin(); shIter != shellIndices.end(); ++shIter) {
            int index = *shIter;
            std::unordered_set<UvEdge, hash_edge>& tempSet = uvShellArrayMaster[index].unordered_edgeSet;
            shellArray.push_back(tempSet);
        }

        if (multiThread) {
            // Multithread

            int threadCount = int(shellArray.size());

            tempResultVector.resize(threadCount);
            for (int i = 0; i < threadCount; i++) {
                tempResultVector[i].reserve(shellArray[i].size());
            }

            std::thread* threadArray = new std::thread[threadCount];

            for (int i = 0; i < threadCount; i++) {
                threadArray[i] = std::thread(
                    &FindUvOverlaps::check,
                    this,
                    std::ref(shellArray[i]),
                    i);
            }

            for (int i = 0; i < threadCount; i++) {
                threadArray[i].join();
            }

            delete[] threadArray;

        } else {
            // Single thread
            tempResultVector.resize(1);
            tempResultVector[0].reserve(1000);
            for (size_t s = 0; s < shellArray.size(); s++) {
                status = check(shellArray[s], 0);
                if (status != MS::kSuccess) {
                    MGlobal::displayInfo("Error found in shell");
                }
            }
        }
    }

    for (size_t i = 0; i < tempResultVector.size(); i++) {
        std::vector<std::string> pathArray = tempResultVector[i];
        for (size_t s = 0; s < pathArray.size(); s++) {
            MString v = pathArray[s].c_str();
            resultStringArray.append(v);
        }
    }

    timer.endTimer();
    if (verbose) {
        timeStr.set(timer.elapsedTime());
        MGlobal::displayInfo("check completed : " + timeStr + " seconds.");
    }
    timer.clear();

    MPxCommand::setResult(resultStringArray);

    return MS::kSuccess;
}

MStatus FindUvOverlaps::initializeObject(const MDagPath& dagPath, const int objectId)
{

    MStatus status;
    MTimer timer;

    MFnMesh fnMesh(dagPath);

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
        currentShell.uVector.push_back(u);
        currentShell.vVector.push_back(v);
    }

    // Setup bounding box information for each shell
    for (unsigned int id = 0; id < nbUvShells; id++) {
        UvShell& shell = uvShellArrayTemp[id];
        shell.uMax = *std::max_element(shell.uVector.begin(), shell.uVector.end());
        shell.vMax = *std::max_element(shell.vVector.begin(), shell.vVector.end());
        shell.uMin = *std::min_element(shell.uVector.begin(), shell.uVector.end());
        shell.vMin = *std::min_element(shell.vVector.begin(), shell.vVector.end());
    }
    
    int numPolygons = fnMesh.numPolygons();
    
    // Create 2d vector of 2d vecotr, which contains Uv indices of each polygon face.
    std::vector<std::vector<int> > uvIdVector;
    uvIdVector.resize(numPolygons);
    MIntArray uvCounts;
    MIntArray uvIds;
    int uvCounter = 0;
    fnMesh.getAssignedUVs(uvCounts, uvIds, uvSetPtr);
    for (unsigned int i=0; i<uvCounts.length(); i++) {
        int numFaceUVs = uvCounts[i];
        for (int u=0; u<numFaceUVs; u++) {
            uvIdVector[i].push_back(uvIds[uvCounter]);
            uvCounter++;
        }
    }
    
    // timer.beginTimer();
    // Loop all polygon faces and create edge objects
    for (int faceId = 0; faceId < numPolygons; faceId++) {
        int numPolygonVertices = uvCounts[faceId];

        if (numPolygonVertices == 0)
            // if current polygon doesn't have mapped UVs, go to next polygon
            continue;
        
        for (int localVtx = 0; localVtx < numPolygonVertices; localVtx++) {
            int curLocalIndex;
            int nextLocalIndex;
            if (localVtx == numPolygonVertices - 1) {
                curLocalIndex = localVtx;
                nextLocalIndex = 0;
            } else {
                curLocalIndex = localVtx;
                nextLocalIndex = localVtx + 1;
            }

            // UV indices by local order
            int uvIdA = uvIdVector[faceId][curLocalIndex];
            int uvIdB = uvIdVector[faceId][nextLocalIndex];
            int currentShellIndex = uvShellIds[uvIdA];

            // Create edge index from two point index and objectID
            // eg. obj1 (1), p1(0), p2(25) makes edge index of 1025
            std::string stringId;
            if (uvIdA < uvIdB) {
                stringId = std::to_string(objectId) + std::to_string(uvIdA) + std::to_string(uvIdB);
            } else {
                stringId = std::to_string(objectId) + std::to_string(uvIdB) + std::to_string(uvIdA);
            }
            
            std::string dagPathStr = dagPath.fullPathName().asChar();
            std::string path_to_p1 = dagPathStr + ".map[" + std::to_string(uvIdA) + "]";
            std::string path_to_p2 = dagPathStr + ".map[" + std::to_string(uvIdB) + "]";
            
            UvPoint p1(uArray[uvIdA], vArray[uvIdA], uvIdA, currentShellIndex, path_to_p1);
            UvPoint p2(uArray[uvIdB], vArray[uvIdB], uvIdB, currentShellIndex, path_to_p2);

            // Create edge objects and insert them to shell edge set
            if (p1 > p2) {
                UvEdge edge(p2, p1, stringId);
                uvShellArrayTemp[currentShellIndex].unordered_edgeSet.insert(edge);
            } else {
                UvEdge edge(p1, p2, stringId);
                uvShellArrayTemp[currentShellIndex].unordered_edgeSet.insert(edge);
            }
        }
    }
    // timer.endTimer();
    // double t = timer.elapsedTime();
    // std::cout << "time : " << t << std::endl;

    // Switch back to the initial uv set
    fnMesh.setCurrentUVSetName(currentUvSet);

    std::copy(uvShellArrayTemp.begin(), uvShellArrayTemp.end(), std::back_inserter(uvShellArrayMaster));

    return MS::kSuccess;
}

MStatus FindUvOverlaps::check(const std::unordered_set<UvEdge, hash_edge>& edges, int threadNumber)
{
    std::deque<Event> eventQueue;

    int eventIndex = 0;
    for (std::unordered_set<UvEdge, hash_edge>::iterator iter = edges.begin(), end = edges.end(); iter != end; ++iter) {
        
        Event ev1("begin", &(*iter), iter->begin, eventIndex);
        eventIndex += 1;
        Event ev2("end", &(*iter), iter->end, eventIndex);
        eventIndex += 1;
        
        eventQueue.push_back(ev1);
        eventQueue.push_back(ev2);
    }
    std::sort(eventQueue.begin(), eventQueue.end());

    std::vector<UvEdge> statusQueue;
    statusQueue.reserve(edges.size());

    while (true) {
        if (eventQueue.empty()) {
            break;
        }
        Event firstEvent = eventQueue.front();
        eventQueue.pop_front();

        if (firstEvent.status == "begin") {
            doBegin(firstEvent, eventQueue, statusQueue, threadNumber);
        } else if (firstEvent.status == "end") {
            doEnd(firstEvent, eventQueue, statusQueue, threadNumber);
        } else if (firstEvent.status == "intersect") {
            doCross(firstEvent, eventQueue, statusQueue, threadNumber);
        } else {
            if (verbose)
                MGlobal::displayError("Unknow exception");
            return MS::kFailure;
        }
    }

    return MS::kSuccess;
}

bool FindUvOverlaps::doBegin(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue, int threadNumber)
{
    const UvEdge& edge = *(currentEvent.edgePtr);
    statusQueue.push_back(edge);

    // if there are no edges to compare
    size_t numStatus = statusQueue.size();
    if (numStatus == 1) {
        return false;
    }

    // Update x values of intersection to the sweepline for all edges
    // in the statusQueue
    float Y = currentEvent.v;
    for (size_t i = 0; i < numStatus; i++) {
        // statusQueue[i].setCrossingPointX(currentEvent.v);
        UvEdge& edge = statusQueue[i];
        if (edge.end.v == edge.begin.v) {
            edge.crossingPointX = edge.begin.u;
        } else {
            float X = ((Y - edge.begin.u) * (edge.end.u - edge.begin.u)) / (edge.end.v - edge.begin.v) + edge.begin.u;
            edge.crossingPointX = X;
        }
    }
    std::sort(statusQueue.begin(), statusQueue.end(), UvEdgeComparator());

    std::vector<UvEdge>::iterator foundIter = std::find(statusQueue.begin(), statusQueue.end(), edge);
    if (foundIter == statusQueue.end()) {
        // not found
        return false;
    }
    size_t index = std::distance(statusQueue.begin(), foundIter);
    if (index == numStatus) {
        // invalid
    }

    UvEdge& currentEdge = statusQueue[index];

    if (index == 0) {
        // If first item, check the next edge
        UvEdge& nextEdge = statusQueue[index + 1];
        checkEdgesAndCreateEvent(currentEdge, nextEdge, eventQueue, threadNumber);
    } else if (index == numStatus - 1) {
        // if last iten in the statusQueue
        UvEdge& previousEdge = statusQueue[index - 1];
        checkEdgesAndCreateEvent(currentEdge, previousEdge, eventQueue, threadNumber);
    } else {
        UvEdge& nextEdge = statusQueue[index + 1];
        UvEdge& previousEdge = statusQueue[index - 1];
        checkEdgesAndCreateEvent(currentEdge, nextEdge, eventQueue, threadNumber);
        checkEdgesAndCreateEvent(currentEdge, previousEdge, eventQueue, threadNumber);
    }
    return true;
}

bool FindUvOverlaps::doEnd(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue, int threadNumber)
{
    const UvEdge& edge = *(currentEvent.edgePtr);
    std::vector<UvEdge>::iterator iter_for_removal = std::find(statusQueue.begin(), statusQueue.end(), edge);
    if (iter_for_removal == statusQueue.end()) {
        if (verbose)
            MGlobal::displayInfo("Failed to find the edge to be removed at the end event.");
        // if iter not found
        // return MS::kFailure;
        return false;
    }

    size_t removeIndex = std::distance(statusQueue.begin(), iter_for_removal);
    if (removeIndex == statusQueue.size()) {
        MGlobal::displayInfo("error2");
        // invalid
        return MS::kFailure;
        return false;
    }

    if (statusQueue.size() <= 2) {
        // if num items are less than 2 in the countainer, do nothing
    } else if (removeIndex == 0) {
        // if first item, do nothing

    } else if (removeIndex == statusQueue.size() - 1) {
        // if last item, do nothing
    } else {
        // check previous and next edge intersection as they can be next
        // each other after removing the current edge
        UvEdge& nextEdge = statusQueue[removeIndex + 1];
        UvEdge& previousEdge = statusQueue[removeIndex - 1];
        checkEdgesAndCreateEvent(previousEdge, nextEdge, eventQueue, threadNumber);
    }

    // Remove current edge from the statusQueue
    statusQueue.erase(iter_for_removal);
    return true;
}

bool FindUvOverlaps::doCross(Event& currentEvent, std::deque<Event>& eventQueue, std::vector<UvEdge>& statusQueue, int threadNumber)
{
    if (statusQueue.size() <= 2) {
        return false;
    }

    const UvEdge& thisEdge = *(currentEvent.edgePtr);
    const UvEdge& otherEdge = *(currentEvent.otherEdgePtr);
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
        UvEdge& firstEdge = statusQueue[small];
        UvEdge& secondEdge = statusQueue[big + 1];
        checkEdgesAndCreateEvent(firstEdge, secondEdge, eventQueue, threadNumber);
    } else if (big == statusQueue.size() - 1) {
        UvEdge& firstEdge = statusQueue[small - 1];
        UvEdge& secondEdge = statusQueue[big];
        checkEdgesAndCreateEvent(firstEdge, secondEdge, eventQueue, threadNumber);
    } else {
        UvEdge& firstEdge = statusQueue[small - 1];
        UvEdge& secondEdge = statusQueue[small];
        UvEdge& thirdEdge = statusQueue[big];
        UvEdge& forthEdge = statusQueue[big + 1];

        checkEdgesAndCreateEvent(firstEdge, thirdEdge, eventQueue, threadNumber);
        checkEdgesAndCreateEvent(secondEdge, forthEdge, eventQueue, threadNumber);
    }
    return false;
}

MStatus FindUvOverlaps::checkEdgesAndCreateEvent(UvEdge& edgeA, UvEdge& edgeB, std::deque<Event>& eventQueue, int threadNumber)
{
    bool isParallel = false;
    if (edgeA.isIntersected(edgeB, isParallel)) {

        float uv[2];
        UvUtils::getEdgeIntersectionPoint(edgeA.begin.u, edgeA.begin.v, edgeA.end.u, edgeA.end.v, edgeB.begin.u, edgeB.begin.v, edgeB.end.u, edgeB.end.v, uv);

        tempResultVector[threadNumber].push_back(edgeA.begin.path);
        tempResultVector[threadNumber].push_back(edgeB.begin.path);
        tempResultVector[threadNumber].push_back(edgeA.end.path);
        tempResultVector[threadNumber].push_back(edgeB.end.path);

        if (isParallel == false) {
            Event crossEvent("intersect", uv[0], uv[1], &edgeA, &edgeB);
            eventQueue.push_back(crossEvent);
            std::sort(eventQueue.begin(), eventQueue.end());
        }
    }
    return MS::kSuccess;
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
