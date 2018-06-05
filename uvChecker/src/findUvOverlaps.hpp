#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MTimer.h>

#include "event.hpp"
#include "uvEdge.hpp"
#include "uvShell.hpp"

#include <set>
#include <vector>

struct objectData {
    int objectId;
    MIntArray* uvCounts;
    MIntArray* uvShellIds;
    std::vector<std::vector<int>>* uvIdVector;
    MFloatArray* uArray;
    MFloatArray* vArray;
    int begin;
    int end;
    int threadIndex;
};

struct checkThreadData {
    Event* currentEventPtr;
    std::multiset<Event>* eventQueuePtr;
    std::vector<UvEdge>* statusQueuePtr;
    int threadNumber;
    float sweepline;
    
    const UvEdge* currentEdgePtr;
    const UvEdge* otherEdgePtr;
    const UvEdge* edgeA;
    const UvEdge* edgeB;
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
    MStatus checkEdgesAndCreateEvent(checkThreadData& checkData);
    MStatus initializeObject(const MDagPath& dagPath, const int objectId);
    MStatus initializeFaces(objectData data, std::vector<std::vector<UvEdge>>& edgeVectorTemp);

    bool doBegin(checkThreadData& checkData);
    bool doEnd(checkThreadData& checkData);
    bool doCross(checkThreadData& checkData);

private:
    bool verbose;
    bool multiThread;
    MDagPath dagPath;
    MString uvSet;
    MSelectionList mSel;
    int numEdges;
    MTimer timer;

    // Container to store all UV shells to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // Countainer for UVs of final result to be returned
    MStringArray resultStringArray;

    // temp result container for each thread
    std::vector<std::vector<std::string>> tempResultVector;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
