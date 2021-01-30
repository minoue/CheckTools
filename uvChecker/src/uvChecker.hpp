#pragma once


#include <maya/MFnMesh.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <vector>

enum class UVCheckType {
    UDIM = 0,
    HAS_UVS,
    ZERO_AREA,
    UN_ASSIGNED_UVS,
    NEGATIVE_SPACE_UVS,
    CONCAVE_UVS
};

class UvChecker final : public MPxCommand {
public:
    UvChecker();
    virtual ~UvChecker();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creator();
    static MSyntax newSyntax();

    using Index = int;
    using IndexArray = std::vector<Index>;

    IndexArray findUdimIntersections(const MFnMesh&);
    IndexArray findNoUvFaces(const MFnMesh&);
    IndexArray findZeroUvFaces(const MFnMesh&);
    IndexArray findNegativeSpaceUVs(const MFnMesh&);
    IndexArray findConcaveUVs(const MFnMesh&);
    bool hasUnassignedUVs(const MFnMesh&);

private:
    bool verbose;
    double minUVArea;
    MString uvSet;
    double maxUvBorderDistance;
    float getTriangleArea(float Ax, float Ay, float Bx, float By, float Cx, float Cy) const;
};
