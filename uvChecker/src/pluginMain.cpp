#include "findUvOverlaps_old.h"
#include "findUvOverlaps.h"
#include "uvChecker.h"
#include <maya/MFnPlugin.h>

static const char* const VERSION = "1.0.2";

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin ovPlugin(mObj, "Michitaka Inoue", VERSION, "Any");
    ovPlugin.registerCommand("findUvOverlaps_old", FindUvOverlaps_Old::creator, FindUvOverlaps_Old::newSyntax);

    MFnPlugin ov2Plugin(mObj, "Michitaka Inoue", VERSION, "Any");
    ovPlugin.registerCommand("findUvOverlaps", FindUvOverlaps::creator, FindUvOverlaps::newSyntax);

    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", VERSION, "Any");
    fnPlugin.registerCommand("checkUV", UvChecker::creator, UvChecker::newSyntax);

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin ovPlugin(mObj);
    ovPlugin.deregisterCommand("findUvOverlaps_old");
    MFnPlugin ov2Plugin(mObj);
    ovPlugin.deregisterCommand("findUvOverlaps");
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("checkUV");

    return MS::kSuccess;
}
