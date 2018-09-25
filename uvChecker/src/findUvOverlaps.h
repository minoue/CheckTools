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

private:
    bool verbose;
    MDagPath dagPath;
    MString uvSet;
    MSelectionList mSel;
    MTimer timer;

    void makeCombinations(size_t N, std::vector<std::vector<int>>& vec);
    void displayTime(std::string message, double time);
    MStatus initializeObject(const MDagPath& dagPath);
    bool isBoundingBoxOverlapped(
        const float BA_uMin,
        const float BA_uMax,
        const float BA_vMin,
        const float BA_vMax,
        const float BB_uMin,
        const float BB_uMax,
        const float BB_vMin,
        const float BB_vMax);
    
    // Container to store all UV shells from all selected objects to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // Countainer for UVs of final result to be returned
    MStringArray resultStringArray;

    // temp result container for each thread
    std::vector<std::string> resultVector;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
