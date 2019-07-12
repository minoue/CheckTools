#ifndef __UVCHECKER_H__
#define __UVCHECKER_H__

#include <maya/MDagPath.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MString.h>

class UvChecker : public MPxCommand {
public:
    UvChecker();
    virtual ~UvChecker();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();

    MStatus findUdimIntersections();
    MStatus findNoUvFaces();
    MStatus findZeroUvFaces();
    bool hasUnassignedUVs();
    MStatus findNegativeSpaceUVs();

    enum Check {
        UDIM,
        HAS_UVS,
        ZERO_AREA,
        UN_ASSIGNED_UVS,
        NEGATIVE_SPACE_UVS
    };

private:
    MDagPath mDagPath;
    bool verbose;
    double minUVArea;
    MString uvSet;
    unsigned int checkNumber;
};

#endif /* defined(__UVCHECKER_H__) */
