#include "uvChecker.hpp"
#include <maya/MFnPlugin.h>
#include <string>

static const char* const pluginCommandName = "checkUV";
static const char* const pluginVersion = "1.9.8";
static const char* const pluginAuthor = "Michi Inoue";

MStatus initializePlugin(MObject mObj)
{
    MStatus status;

    std::string version_str(pluginVersion);
    std::string compile_date_str(__DATE__);
    std::string compile_time_str(__TIME__);
    std::string version(version_str + " / " + compile_date_str + " / " + compile_time_str);

    MFnPlugin fnPlugin(mObj, pluginAuthor, version.c_str(), "Any");

    status = fnPlugin.registerCommand(pluginCommandName, UvChecker::creator, UvChecker::newSyntax);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject mObj)
{
    MStatus status;

    MFnPlugin fnPlugin(mObj);
    status = fnPlugin.deregisterCommand(pluginCommandName);
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    return MS::kSuccess;
}
