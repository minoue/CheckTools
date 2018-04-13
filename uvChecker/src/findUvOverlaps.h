#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>

#include "event.h"
#include "uvEdge.h"
#include "uvShell.h"

#include <unordered_set>
#include <vector>


struct objectData {
    int faceId;
    int objectId;
    MIntArray* uvCounts;
    MIntArray* uvShellIds;
    std::vector<std::vector<int>>* uvIdVector;
    MFloatArray* uArray;
    MFloatArray* vArray;
};

class FindUvOverlaps : public MPxCommand {
public:
    FindUvOverlaps();
    virtual ~FindUvOverlaps();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();

    MStatus check(const std::unordered_set<UvEdge, hash_edge>& edges, int threadNumber);
    MStatus checkEdgesAndCreateEvent(UvEdge& edgeA, UvEdge& edgeB, std::vector<Event>& eventQueue, int threadNumber);
    MStatus initializeObject(const MDagPath& dagPath, const int objectId);
    MStatus initializeFace(objectData data, std::vector<UvShell>& result);

    bool doBegin(Event& currentEvent,
        std::vector<Event>& eventQueue,
        std::vector<UvEdge>& statusQueue,
        int threadNumber);
    bool doEnd(Event& currentEvent,
        std::vector<Event>& eventQueue,
        std::vector<UvEdge>& statusQueue,
        int threadNumber);
    bool doCross(Event& currentEvent,
        std::vector<Event>& eventQueue,
        std::vector<UvEdge>& statusQueue,
        int threadNumber);

private:
    bool verbose;
    bool multiThread;
    MDagPath dagPath;
    MString uvSet;
    MSelectionList mSel;

    // Container to store all UV shells to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // Countainer for UVs of final result to be returned 
    MStringArray resultStringArray;

    // temp result container for each thread
    std::vector<std::vector<std::string>> tempResultVector;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
