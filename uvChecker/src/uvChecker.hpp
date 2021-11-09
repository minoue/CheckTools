#pragma once

#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

enum class UVCheckType {
    UDIM = 0,
    HAS_UVS,
    ZERO_AREA,
    UN_ASSIGNED_UVS,
    NEGATIVE_SPACE_UVS,
    CONCAVE_UVS,
    REVERSED_UVS
};

class UvChecker final : public MPxCommand {
public:
    UvChecker();
    ~UvChecker() final;

    // command interface
    MStatus doIt(const MArgList& argList) final;
    MStatus undoIt() final;
    MStatus redoIt() final;
    bool isUndoable() const final;

    static void* creator();
    static MSyntax newSyntax();

private:
    bool verbose;
    double minUVArea;
    MString uvSet;
    double maxUvBorderDistance;
};
