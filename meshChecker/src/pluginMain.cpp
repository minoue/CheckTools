#include "meshChecker.h"
#include <maya/MFnPlugin.h>

static const char* const VERSION = "1.2.0";

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", VERSION, "Any");
    fnPlugin.registerCommand("checkMesh", MeshChecker::creator, MeshChecker::newSyntax);
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("checkMesh");
    return MS::kSuccess;
}
