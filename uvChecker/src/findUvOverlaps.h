#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MTimer.h>

#include "bentleyOttman/event.h"
#include "bentleyOttman/uvEdge.h"
#include "uvShell.h"

#include <mutex>
#include <set>
#include <utility>
#include <vector>

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

    MStatus check(const std::set<UvEdge>& edges, int threadNumber);
    MStatus checkEdgesAndCreateEvent(checkThreadData& checkData);
    MStatus initializeObject(const MDagPath& dagPath, const int objectId);

    bool doBegin(checkThreadData& checkData);
    bool doEnd(checkThreadData& checkData);
    bool doCross(checkThreadData& checkData);

    void safeInsert(const std::string& path);

private:
    bool verbose;
    MDagPath dagPath;
    MString uvSet;
    MSelectionList mSel;
    int numEdges;
    MTimer timer;
    std::mutex mtx;

    // Container to store all UV shells from all selected objects to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // Countainer for UVs of final result to be returned
    MStringArray resultStringArray;

    // temp result container for each thread
    std::vector<std::string> resultVector;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
