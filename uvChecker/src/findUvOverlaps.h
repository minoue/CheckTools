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

#include "uvShell.h"

#include <mutex>
#include <set>
#include <utility>
#include <vector>

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

    MStatus initializeObject(const MDagPath& dagPath);

private:
    bool verbose;
    MDagPath dagPath;
    MString uvSet;
    MSelectionList mSel;
    MTimer timer;

    // Container to store all UV shells from all selected objects to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // Countainer for UVs of final result to be returned
    MStringArray resultStringArray;

    // temp result container for each thread
    std::vector<std::string> resultVector;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
