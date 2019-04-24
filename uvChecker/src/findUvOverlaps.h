#ifndef __FINDUVOVERLAPS2_H__
#define __FINDUVOVERLAPS2_H__

#include <maya/MDagPath.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MSyntax.h>
#include <maya/MTimer.h>
#include <maya/MFnMesh.h>

#include "uvShell.h"

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
    int uvShellCounter;
    bool verbose;
    MDagPath dagPath;
    MString uvSet;
    bool multithread;
    MString currentUVSetName;
    MSelectionList mSel;
    MTimer timer;

    MString getWorkUvSet(MFnMesh& fnMesh);
	MStatus initializeObject(MDagPath& dagPath);
    void check(BentleyOttmann& bto);
    void check_mt(std::vector<BentleyOttmann> &bto, int start, int end);
    void displayTime(std::string message, double time);

    // Container to store all UV shells from all selected objects to be tested
    std::vector<UvShell> uvShellArrayMaster;

    // Countainer for UVs of final result to be returned
    MStringArray resultStringArray;
};

#endif /* defined(__FINDUVOVERLAPS2_H__) */
