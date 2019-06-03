
#include <algorithm>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MTimer.h>

#include "bentleyOttmann/bentleyOttmann.hpp"
#include "bentleyOttmann/lineSegment.hpp"

static const char* pluginName = "findUvOverlaps";
static const char* pluginVersion = "1.7.1";
static const char* pluginAuthor = "Michitaka Inoue";

class UVShell {
    int index;
    float left, right, top, bottom;
public:
    UVShell(){};
    ~UVShell();
    std::vector<LineSegment> lines;
    void initAABB();
    bool operator*(const UVShell& other) const;
    UVShell operator&&(const UVShell& other) const;
};

UVShell::~UVShell(){};

void UVShell::initAABB()
{
    size_t numLines = this->lines.size();
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
    float left, right, top, bottom;

    if (this->left < other.left)
        left = other.left;
    else
        left = this->left;

    if (this->right < other.right)
        right = this->right;
    else
        right = other.right;

    if (this->top < other.top)
        top = this->top;
    else
        top = other.top;

    if (this->bottom < other.bottom)
        bottom = other.bottom;
    else
        bottom = this->bottom;

    std::vector<LineSegment> overlapLines;
    size_t numLinesA = this->lines.size();
    size_t numLinesB = other.lines.size();

    for (int i = 0; i < numLinesA; i++) {
        const LineSegment& line = this->lines[i];
        if ((left <= line.begin.x && line.begin.x <= right) && (bottom <= line.begin.y && line.begin.y <= top)) {
            overlapLines.emplace_back(line);
            continue;
        }
        if ((left <= line.end.x && line.end.x <= right) && (bottom <= line.end.y && line.end.y <= top)) {
            overlapLines.emplace_back(line);
            continue;
        }
    }

    for (size_t j = 0; j < numLinesB; j++) {
        const LineSegment& line = other.lines[j];
        if ((left <= line.begin.x && line.begin.x <= right) && (bottom <= line.begin.y && line.begin.y <= top)) {
            overlapLines.emplace_back(line);
            continue;
        }
        if ((left <= line.end.x && line.end.x <= right) && (bottom <= line.end.y && line.end.y <= top)) {
            overlapLines.emplace_back(line);
            continue;
        }
    }

    UVShell s;
    s.lines = overlapLines;
    return s;
}

class ShellVector {
private:
    std::mutex mtx;
public:
    std::vector<UVShell> shells;
    void emplace_back(UVShell s)
    {
        std::lock_guard<std::mutex> lock(mtx);
        shells.emplace_back(s);
    }
    size_t size()
    {
        return shells.size();
    }
    UVShell getShell(int i)
    {
        return shells[i];
    }
};

class FindUvOverlaps : public MPxCommand {
public:
    FindUvOverlaps(){};
    ~FindUvOverlaps() override;

    MStatus doIt(const MArgList& args) override;

    static void* creator();
    static MSyntax newSyntax();

private:
    MString uvSet;
    bool verbose;
    MSelectionList mSel;
    MStatus init(int i);
    ShellVector allShells;
    std::vector<BentleyOttmann> btoVector;
    void btoCheck(int i);
    void timeIt(std::string text, double t);
};

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

void FindUvOverlaps::btoCheck(int i)
{
    btoVector[i].check();
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
    std::thread* threadArray = new std::thread[mSel.length()];
    for (size_t i = 0; i < mSel.length(); i++) {
        threadArray[i] = std::thread(&FindUvOverlaps::init, this, i);
    }
    for (size_t i = 0; i < mSel.length(); i++) {
        threadArray[i].join();
    }
    delete[] threadArray;
    timer.endTimer();
    elapsedTime = timer.elapsedTime();
    if (verbose)
        timeIt("Init time : ", elapsedTime);
    timer.clear();

    size_t numAllShells = allShells.size();
    btoVector.reserve(numAllShells);

    for (int i = 0; i < numAllShells; i++) {
        UVShell& s = allShells.shells[i];
        s.initAABB();
    }

    for (int i = 0; i < numAllShells; i++) {
        UVShell& shellA = allShells.shells[i];

        // single shell check
        BentleyOttmann bto(shellA.lines);
        btoVector.emplace_back(bto);

        for (int j = i + 1; j < numAllShells; j++) {
            UVShell& shellB = allShells.shells[j];

            if (shellA * shellB) {
                UVShell intersectedShell = shellA && shellB;
                BentleyOttmann intersectedBto(intersectedShell.lines);
                btoVector.emplace_back(intersectedBto);
            }
        }
    }

    timer.beginTimer();
    // Multithread bentleyOttman check
    std::thread* btoThreadArray = new std::thread[numAllShells];
    for (size_t i = 0; i < numAllShells; i++) {
        btoThreadArray[i] = std::thread(&FindUvOverlaps::btoCheck, this, i);
    }
    for (size_t i = 0; i < numAllShells; i++) {
        btoThreadArray[i].join();
    }
    delete[] btoThreadArray;
    timer.endTimer();
    elapsedTime = timer.elapsedTime();
    if (verbose)
        timeIt("Check time : ", elapsedTime);

    // Re-insert ot unordered_set to remove duplicates
    std::set<std::string> resultSet;
    std::string path;
    for (size_t i = 0; i < btoVector.size(); i++) {

        BentleyOttmann& bto = btoVector[i];
        std::vector<LineSegment*>::iterator iter;

        for (iter = bto.resultPtr.begin(); iter != bto.resultPtr.end(); ++iter) {
            LineSegment* linePtr = *iter;
            const LineSegment& line = *linePtr;

            path = line.groupId + ".map[" + std::to_string(line.index.first) + "]";
            resultSet.insert(path);

            path = line.groupId + ".map[" + std::to_string(line.index.second) + "]";
            resultSet.insert(path);
        }
    }

    // Insert all results to MStringArray for return
    MString s;
    MStringArray resultStringArray;
    std::set<std::string>::iterator resultSetIter;
    for (resultSetIter = resultSet.begin(); resultSetIter != resultSet.end(); ++resultSetIter) {
        s.set((*resultSetIter).c_str());
        resultStringArray.append(s);
    }

    setResult(resultStringArray);

    return MS::kSuccess;
}

MStatus FindUvOverlaps::init(int i)
{

    MDagPath dagPath;
    mSel.getDagPath(i, dagPath);
    MFnMesh fnMesh(dagPath);
    std::string dagPathStr = dagPath.fullPathName().asChar();

    MIntArray uvShellIds;
    unsigned int nbUvShells;
    fnMesh.getUvShellsIds(uvShellIds, nbUvShells);

    MIntArray uvCounts; // Num of UVs per face eg. [4, 4, 4, 4, ...]
    MIntArray uvIds;
    fnMesh.getAssignedUVs(uvCounts, uvIds);

    size_t uvCountSize = uvCounts.length(); // is same as number of faces
    std::vector<std::pair<int, int>> idPairs;
    idPairs.reserve(uvCountSize * 4);
    int uvCounter = 0;
    int nextCounter;
    // Loop over each face and its edges, then create a pair of indices
    for (unsigned int j = 0; j < uvCountSize; j++) {
        int numFaceUVs = uvCounts[j];
        for (int localIndex = 0; localIndex < numFaceUVs; localIndex++) {
            if (localIndex == numFaceUVs - 1) {
                // Set the nextCounter to the localIndex of zero of the face
                nextCounter = uvCounter - numFaceUVs + 1;
            } else {
                nextCounter = uvCounter + 1;
            }

            int idA = uvIds[uvCounter];
            int idB = uvIds[nextCounter];

            std::pair<int, int> idPair;

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
    std::vector<std::pair<int, int>>::iterator pairIter;
    for (pairIter = idPairs.begin(); pairIter != idPairs.end(); ++pairIter) {

        int idA = (*pairIter).first;
        int idB = (*pairIter).second;
        Point2D p1(uArray[idA], vArray[idA], idA);
        Point2D p2(uArray[idB], vArray[idB], idB);

        // Create new lineSegment object
        LineSegment line(p1, p2, dagPathStr);

        int shellIndex = uvShellIds[idA];
        shells[shellIndex].lines.emplace_back(line);
    }

    for (int i = 0; i < shells.size(); i++) {
        allShells.emplace_back(shells[i]);
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
