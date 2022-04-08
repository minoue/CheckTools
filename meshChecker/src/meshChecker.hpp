#pragma once

#include <maya/MPxCommand.h>

enum class MeshCheckType {
    TRIANGLES = 0,
    NGONS,
    NON_MANIFOLD_EDGES,
    LAMINA_FACES,
    BI_VALENT_FACES,
    ZERO_AREA_FACES,
    MESH_BORDER,
    CREASE_EDGE,
    ZERO_LENGTH_EDGES,
    UNFROZEN_VERTICES,
    EMPTY_GEOMETRY,
    UNUSED_VERTICES,
    INSTANCE,
    TEST
};

class MeshChecker final : public MPxCommand {
public:
    static void* creator();
    static MSyntax newSyntax();

    // command interface
    MStatus doIt(const MArgList& argList) final;
    MStatus undoIt() final;
    MStatus redoIt() final;
    bool isUndoable() const final;

private:
    MeshChecker();
};
