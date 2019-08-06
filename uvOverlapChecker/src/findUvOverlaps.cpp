#include <algorithm>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <unordered_set>
#include "findUvOverlaps.h"
#include <maya/MArgDatabase.h>
#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MStringArray.h>
#include <maya/MTimer.h>

static const char* pluginName = "findUvOverlaps";
static const char* pluginVersion = "1.8.11";
static const char* pluginAuthor = "Michitaka Inoue";

void UVShell::initAABB()
{
    std::vector<float> uVector;
    std::vector<float> vVector;
    std::vector<LineSegment>::iterator lineIter;
    for (lineIter = this->lines.begin(); lineIter != this->lines.end(); ++lineIter) {
        uVector.emplace_back(lineIter->begin.x);
        uVector.emplace_back(lineIter->end.x);
        vVector.emplace_back(lineIter->begin.y);
        vVector.emplace_back(lineIter->end.y);
    }
    this->left = *std::min_element(uVector.begin(), uVector.end());
    this->right = *std::max_element(uVector.begin(), uVector.end());
    this->bottom = *std::min_element(vVector.begin(), vVector.end());
    this->top = *std::max_element(vVector.begin(), vVector.end());
}

bool UVShell::operator*(const UVShell& other) const
{
    if (this->right < other.left)
        return false;

    if (this->left > other.right)
        return false;

    if (this->top < other.bottom)
        return false;

    if (this->bottom > other.top)
        return false;

    return true;
}

UVShell UVShell::operator&&(const UVShell& other) const
{
    float L, R, T, B;

    if (this->left < other.left)
        L = other.left;
    else
        L = this->left;

    if (this->right < other.right)
        R = this->right;
    else
        R = other.right;

    if (this->top < other.top)
        T = this->top;
    else
        T = other.top;

    if (this->bottom < other.bottom)
        B = other.bottom;
    else
        B = this->bottom;

    std::vector<LineSegment> overlapLines;
    size_t numLinesA = this->lines.size();
    size_t numLinesB = other.lines.size();

    for (size_t i = 0; i < numLinesA; i++) {
        const LineSegment& line = this->lines[i];
        if ((L <= line.begin.x && line.begin.x <= R) && (B <= line.begin.y && line.begin.y <= T)) {
            overlapLines.emplace_back(line);
            continue;
        }
        if ((L <= line.end.x && line.end.x <= R) && (B <= line.end.y && line.end.y <= T)) {
            overlapLines.emplace_back(line);
            continue;
        }
    }

    for (size_t j = 0; j < numLinesB; j++) {
        const LineSegment& line = other.lines[j];
        if ((L <= line.begin.x && line.begin.x <= R) && (B <= line.begin.y && line.begin.y <= T)) {
            overlapLines.emplace_back(line);
            continue;
        }
        if ((L <= line.end.x && line.end.x <= R) && (B <= line.end.y && line.end.y <= T)) {
            overlapLines.emplace_back(line);
            continue;
        }
    }

    UVShell s;
    s.lines = overlapLines;
    return s;
}


FindUvOverlaps::~FindUvOverlaps() {}

void* FindUvOverlaps::creator()
{
    return new FindUvOverlaps();
}

MSyntax FindUvOverlaps::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-set", "-uvSet", MSyntax::kString);
    return syntax;
}

void FindUvOverlaps::btoCheck(UVShell &shell)
{
    std::vector<LineSegment> result;
    BentleyOttmann b(shell.lines);
    b.check(result);
    pushToLineVector(result);
}

void FindUvOverlaps::pushToLineVector(std::vector<LineSegment> &v)
{
    try {
        locker.lock();
        finalResult.push_back(v);
        locker.unlock();
    }
    catch (std::exception e) {
        std::cerr << e.what() << std::endl;
    }
}

void FindUvOverlaps::pushToShellVector(UVShell &shell)
{
    try {
        locker.lock();
        shellVector.push_back(shell);
        locker.unlock();
    }
    catch (std::exception e) {
        std::cerr << e.what() << std::endl;
    }
}

MStatus FindUvOverlaps::doIt(const MArgList& args)
{
    MStatus stat;
    MTimer timer;
    double elapsedTime;

    MArgDatabase argData(syntax(), args);

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);
    else
        verbose = false;

    if (argData.isFlagSet("-uvSet"))
        argData.getFlagArgument("-uvSet", 0, uvSet);
    else
        uvSet = "None";

    MGlobal::getActiveSelectionList(mSel);

    timer.beginTimer();
    // Multithrad obj initialization
    // std::thread* threadArray = new std::thread[mSel.length()];
    // for (size_t i = 0; i < mSel.length(); i++) {
    //     threadArray[i] = std::thread(&FindUvOverlaps::init, this, i);
    // }
    // for (size_t i = 0; i < mSel.length(); i++) {
    //     threadArray[i].join();
    // }
    // delete[] threadArray;

    int numSelected = mSel.length();
    #pragma omp parallel for
    for (int i = 0; i < numSelected; i++) {
        init(i);
    }

    timer.endTimer();
    elapsedTime = timer.elapsedTime();
    if (verbose)
        timeIt("Init time : ", elapsedTime);
    timer.clear();

    size_t numAllShells = shellVector.size();

    for (size_t i = 0; i < numAllShells; i++) {
        UVShell& s = shellVector[i];
        s.initAABB();
    }

    std::vector<UVShell> shells; // temp countainer to store both original and combined shells

    for (size_t i = 0; i < numAllShells; i++) {
        UVShell& shellA = shellVector[i];
        shells.push_back(shellA);

        for (size_t j = i + 1; j < numAllShells; j++) {
            UVShell& shellB = shellVector[j];

            if (shellA * shellB) {
                UVShell intersectedShell = shellA && shellB;
                shells.push_back(intersectedShell);
            }
        }
    }

    timer.beginTimer();

    if (verbose) {
        MString numShellsStr;
        numShellsStr.set(static_cast<int>(shells.size()));
        MGlobal::displayInfo("Number of UvShells : " + numShellsStr);
    }

    // Multithread bentleyOttman check
    size_t numAllShells2 = shells.size();
    std::thread* btoThreadArray = new std::thread[numAllShells2];
    for (size_t i = 0; i < numAllShells2; i++) {
        UVShell &s = shells[i];
        btoThreadArray[i] = std::thread(&FindUvOverlaps::btoCheck, this, std::ref(s));
    }
    for (size_t i = 0; i < numAllShells2; i++) {
        btoThreadArray[i].join();
    }
    delete[] btoThreadArray;

    timer.endTimer();
    elapsedTime = timer.elapsedTime();
    if (verbose)
        timeIt("Check time : ", elapsedTime);
    timer.clear();

    timer.beginTimer();
    // Re-insert to set to remove duplicates
    std::string temp_path;
    std::unordered_set<std::string> temp;
    for (auto&& lines : finalResult) {
        for (auto&& line : lines) {
            std::string groupName(line.groupId);
            temp_path = groupName + ".map[" + std::to_string(line.index.first) + "]";
            temp.insert(temp_path);
            temp_path = groupName + ".map[" + std::to_string(line.index.second) + "]";
            temp.insert(temp_path);
        }
    }

    // Remove duplicates
    timer.endTimer();
    elapsedTime = timer.elapsedTime();
    if (verbose)
        timeIt("Removed duplicates : ", elapsedTime);
    timer.clear();

    // Insert all results to MStringArray for return
    MString s;
    MStringArray resultStringArray;

    for (auto fullpath : temp) {
        s.set(fullpath.c_str());
        resultStringArray.append(s);
    }

    setResult(resultStringArray);

    return MS::kSuccess;
}

MStatus FindUvOverlaps::init(int i)
{
    MStatus status;

    MDagPath dagPath;
    mSel.getDagPath(static_cast<unsigned int>(i), dagPath);

    // Check if specified object is geometry or not
    status = dagPath.extendToShape();
    if (status != MS::kSuccess) {
        if (verbose)
            MGlobal::displayInfo("Failed to extend to shape node.");
        return MS::kFailure;
    }

    if (dagPath.apiType() != MFn::kMesh) {
        if (verbose)
            MGlobal::displayInfo("Selected node : " + dagPath.fullPathName() + " is not mesh. Skipped");
        return MS::kFailure;
    }

    MFnMesh fnMesh(dagPath);

    // Send to path vector and get pointer to that
    const char* dagPathChar = paths.emplace_back(dagPath.fullPathName());

    MIntArray uvShellIds;
    unsigned int nbUvShells;
    fnMesh.getUvShellsIds(uvShellIds, nbUvShells);

    MIntArray uvCounts; // Num of UVs per face eg. [4, 4, 4, 4, ...]
    MIntArray uvIds;
    fnMesh.getAssignedUVs(uvCounts, uvIds);

    unsigned int uvCountSize = uvCounts.length(); // is same as number of faces
    std::vector<std::pair<unsigned int, unsigned int>> idPairs;
    idPairs.reserve(uvCountSize * 4);
    unsigned int uvCounter = 0;
    unsigned int nextCounter;
    // Loop over each face and its edges, then create a pair of indices
    for (unsigned int j = 0; j < uvCountSize; j++) {
        unsigned int numFaceUVs = static_cast<unsigned int>(uvCounts[j]);
        for (unsigned int localIndex = 0; localIndex < numFaceUVs; localIndex++) {
            if (localIndex == numFaceUVs - 1) {
                // Set the nextCounter to the localIndex of zero of the face
                nextCounter = uvCounter - numFaceUVs + 1;
            } else {
                nextCounter = uvCounter + 1;
            }

            int idA = uvIds[uvCounter];
            int idB = uvIds[nextCounter];

            std::pair<unsigned int, unsigned int> idPair;

            if (idA < idB)
                idPair = std::make_pair(idA, idB);
            else
                idPair = std::make_pair(idB, idA);

            idPairs.emplace_back(idPair);
            uvCounter++;
        }
    }

    // Remove duplicate elements
    std::sort(idPairs.begin(), idPairs.end());
    idPairs.erase(std::unique(idPairs.begin(), idPairs.end()), idPairs.end());

    // Temp countainer for lineSegments for each UVShell
    std::vector<std::vector<LineSegment>> edgeVector;
    edgeVector.resize(nbUvShells);

    MFloatArray uArray;
    MFloatArray vArray;
    fnMesh.getUVs(uArray, vArray);

    // Setup uv shell objects
    std::vector<UVShell> shells(nbUvShells);

    // Loop over all id pairs and create lineSegment objects
    std::vector<std::pair<unsigned int, unsigned int>>::iterator pairIter;
    for (pairIter = idPairs.begin(); pairIter != idPairs.end(); ++pairIter) {

        unsigned int idA = (*pairIter).first;
        unsigned int idB = (*pairIter).second;
        Point2D p1(uArray[idA], vArray[idA], static_cast<int>(idA));
        Point2D p2(uArray[idB], vArray[idB], static_cast<int>(idB));

        // Create new lineSegment object
        LineSegment line(p1, p2, dagPathChar);

        unsigned int shellIndex = static_cast<unsigned int>(uvShellIds[idA]);
        shells[shellIndex].lines.emplace_back(line);
    }

    for (size_t j = 0; j < shells.size(); j++) {
        pushToShellVector(shells[j]);
    }

    return MS::kSuccess;
}

void FindUvOverlaps::timeIt(std::string text, double t)
{
    MString message, time;
    message.set(text.c_str());
    time.set(t);
    MGlobal::displayInfo(message + " : " + time + " seconds.");
}

//
// The following routines are used to register/unregister
// the command we are creating within Maya
//

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, pluginAuthor, pluginVersion, "Any");

    status = plugin.registerCommand(pluginName, FindUvOverlaps::creator, FindUvOverlaps::newSyntax);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand(pluginName);
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    return status;
}
