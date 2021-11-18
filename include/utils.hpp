#pragma once

#include <string>
#include <vector>

#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MItDag.h>


inline float getTriangleArea(float Ax, float Ay, float Bx, float By, float Cx, float Cy)
{
    return ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) * 0.5F;
}

enum class ResultType {
    Face,
    Vertex,
    Edge,
    UV
};

inline void createResultString(const MDagPath& dagPath, ResultType type, int index, std::string& outPath)
{
    outPath = std::string(dagPath.fullPathName().asChar());

    switch (type) {
    case ResultType::Face: {
        outPath += ".f[" + std::to_string(index) + "]";
        break;
    }
    case ResultType::Vertex: {
        outPath += ".vtx[" + std::to_string(index) + "]";
        break;
    }
    case ResultType::Edge: {
        outPath += ".e[" + std::to_string(index) + "]";
        break;
    }
    case ResultType::UV: {
        outPath += ".map[" + std::to_string(index) + "]";
        break;
    }
    }
}

void buildHierarchy(const MDagPath& path, std::vector<std::string>& result)
{

    MString name;

    MItDag dagIter;
    for (dagIter.reset(path, MItDag::kDepthFirst); !dagIter.isDone(); dagIter.next()) {
        MObject obj = dagIter.currentItem();

        if (obj.apiType() == MFn::kMesh) {
            name = dagIter.fullPathName();
            result.push_back(name.asChar());
        }
    }
}