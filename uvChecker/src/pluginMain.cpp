#include "uvChecker.h"
#include <maya/MFnPlugin.h>

static const char* const VERSION = "1.8.0";

MStatus initializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj, "Michitaka Inoue", VERSION, "Any");
    fnPlugin.registerCommand("checkUV", UvChecker::creator, UvChecker::newSyntax);

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MFnPlugin fnPlugin(mObj);
    fnPlugin.deregisterCommand("checkUV");

    return MS::kSuccess;
}