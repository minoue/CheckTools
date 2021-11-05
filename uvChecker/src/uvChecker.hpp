#pragma once


#include <maya/MFnMesh.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include <vector>
#include <mutex>
#include <string>

enum class UVCheckType {
    UDIM = 0,
    HAS_UVS,
    ZERO_AREA,
    UN_ASSIGNED_UVS,
    NEGATIVE_SPACE_UVS,
    CONCAVE_UVS,
    REVERSED_UVS
};

class ResultStringArray {
    std::mutex mtx;

public:
    std::vector<std::string> data;
    void push_back(std::string x)
    {
        std::lock_guard<std::mutex> lock(mtx);
        data.push_back(x);
    }
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
