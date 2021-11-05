#include "uvChecker.hpp"
#include "maya/MApiNamespace.h"
#include "maya/MFnMesh.h"
#include "maya/MString.h"
#include <algorithm>
#include <cstddef>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <maya/MItDag.h>

#include <string>
#include <unordered_set>
#include <thread>

// using IndexArray = UvChecker::IndexArray;

namespace {

enum class ResultType {
    Face,
    Vertex,
    Edge,
    UV
};

float getTriangleArea(float Ax, float Ay, float Bx, float By, float Cx, float Cy) {
    return ((Ax * (By - Cy)) + (Bx * (Cy - Ay)) + (Cx * (Ay - By))) * 0.5F;
}

std::string createResultString(const MDagPath& dagPath, ResultType type, int index)
{

    MString tempPath = dagPath.fullPathName();
    std::string pathString = std::string(tempPath.asChar());

    switch (type) {
    case ResultType::Face: {
        pathString += ".f[" + std::to_string(index) + "]";
        break;
    }
    case ResultType::Vertex: {
        pathString += ".vtx[" + std::to_string(index) + "]";
        break;
    }

    case ResultType::Edge: {
        pathString += ".e[" + std::to_string(index) + "]";
        break;
    }
    case ResultType::UV: {
        pathString += ".map[" + std::to_string(index) + "]";
        break;
    }
    }

    return pathString;
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

void findUdimIntersections(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet, const double maxUvBorderDistance)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i=0; i<length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        std::vector<int> indices;
        
        for (MItMeshPolygon mItPoly(dagPath); !mItPoly.isDone(); mItPoly.next()) {
            int vCount = static_cast<int>(mItPoly.polygonVertexCount());
            int currentUVindex;
            int nextUVindex;
            float u1, v1, u2, v2;

            for (int j = 0; j < vCount; j++) {
                mItPoly.getUVIndex(j, currentUVindex, &uvSet);

                if (j == vCount - 1) {
                    mItPoly.getUVIndex(0, nextUVindex, &uvSet);
                } else {
                    mItPoly.getUVIndex(j + 1, nextUVindex, &uvSet);
                }

                mesh.getUV(currentUVindex, u1, v1, &uvSet);
                mesh.getUV(nextUVindex, u2, v2, &uvSet);

                if (floor(u1) == floor(u2) && floor(v1) == floor(v2)) {
                } else if ((maxUvBorderDistance == 0.0)
                    || ((fabs(rint(u1) - fabs(u1)) > maxUvBorderDistance)
                        && (fabs(rint(v1) - fabs(v1)) > maxUvBorderDistance)
                        && (fabs(rint(u2) - fabs(u2)) > maxUvBorderDistance)
                        && (fabs(rint(v2) - fabs(v2)) > maxUvBorderDistance))) {
                    indices.push_back(currentUVindex);
                    indices.push_back(nextUVindex);
                }
            }
        }
        // Remove duplicate elements
        std::sort(indices.begin(), indices.end());
        indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

        for (auto& index: indices) {
            result->push_back(createResultString(dagPath, ResultType::UV, index));
        }
    }
}

void findNoUvFaces(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    bool hasUVs;

    for (unsigned int i=0; i<length; i++) {
        list.getDagPath(i, dagPath);
        for (MItMeshPolygon itPoly(dagPath); !itPoly.isDone(); itPoly.next()) {
            hasUVs = itPoly.hasUVs(uvSet);
            if (!hasUVs) {
                    result->push_back(createResultString(
                        dagPath,
                        ResultType::Face,
                        static_cast<int>(itPoly.index())));
            }
        }
    }
}

void findZeroUvFaces(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet, const double minUVArea)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i=0; i<length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        double area;
        bool hasUVs;
        for (MItMeshPolygon itPoly(dagPath); !itPoly.isDone(); itPoly.next()) {
            hasUVs = itPoly.hasUVs(uvSet);
            if (!hasUVs) {
                //
            } else {
                itPoly.getUVArea(area, &uvSet);
                if (area < minUVArea) {
                    result->push_back(createResultString(
                        dagPath, ResultType::Face,
                        static_cast<int>(itPoly.index())));
                }
            }
        }
    }
}

void hasUnassignedUVs(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i=0; i<length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);

        int numUVs = mesh.numUVs(uvSet);
        MIntArray uvCounts;
        MIntArray uvIds;
        mesh.getAssignedUVs(uvCounts, uvIds, &uvSet);
        unsigned int numUvIds = uvIds.length();

        std::unordered_set<int> uvIdSet;
        for (unsigned int i = 0; i < numUvIds; i++) {
            uvIdSet.insert(uvIds[i]);
        }

        int numAssignedUVs = static_cast<int>(uvIdSet.size());

        if (numUVs != numAssignedUVs) {
            result->push_back(dagPath.fullPathName().asChar());
        }
    }
}

void findNegativeSpaceUVs(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    MFloatArray uArray, vArray;



    for (unsigned int i=0; i<length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        std::vector<int> indices;
        mesh.getUVs(uArray, vArray, &uvSet);

        int numUVs = mesh.numUVs(uvSet);

        for (int j=0; j<numUVs; j++) {
            float u = uArray[static_cast<unsigned int>(j)];
            if (u < 0.0) {
                indices.push_back(j);
                continue;
            }
            float v = vArray[static_cast<unsigned int>(j)];
            if (v < 0.0) {
                indices.push_back(j);
                continue;
            }
        }
        // Remove duplicate elements
        std::sort(indices.begin(), indices.end());
        indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

        for (auto& index: indices) {
            result->push_back(createResultString(dagPath, ResultType::UV, index));
        }
    }
}

void findConcaveUVs(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        // IndexArray indices;
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        std::vector<int> indices;

        MPointArray points;
        MIntArray vertexList;

        for (MItMeshPolygon itPoly(dagPath); !itPoly.isDone(); itPoly.next()) {
            unsigned int polygonIndex = itPoly.index();
            itPoly.getTriangles(points, vertexList);
            int numVerts = static_cast<int>(itPoly.polygonVertexCount());
            int p1, p2, p3;
            bool isReversed = itPoly.isUVReversed();

            for (int i=0; i<numVerts; i++) {
                p1 = i;
                p2 = i + 1;
                p3 = i + 2;

                if (p2 > numVerts - 1) {
                    p2 -= numVerts;
                }
                if (p3 > numVerts - 1) {
                    p3 -= numVerts;
                }

                float2 uv1;
                float2 uv2;
                float2 uv3;

                itPoly.getUV(p1, uv1, &uvSet);
                itPoly.getUV(p2, uv2, &uvSet);
                itPoly.getUV(p3, uv3, &uvSet);

                float S = getTriangleArea(uv1[0], uv1[1], uv2[0], uv2[1], uv3[0], uv3[1]);

                if (!isReversed) {
                    if (S <= 0.00000000001) {
                        indices.push_back(static_cast<int>(polygonIndex));
                    }
                } else {
                    if (S >= -0.00000000001) {
                        indices.push_back(static_cast<int>(polygonIndex));
                    }
                }
            }
        }
        // Remove duplicate elements
        std::sort(indices.begin(), indices.end());
        indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

        for (auto& index: indices) {
            result->push_back(createResultString(dagPath, ResultType::Face, index));
        }
    }
}

void findReversedUVs(std::vector<std::string>* paths, ResultStringArray* result, const MString uvSet)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        for (MItMeshPolygon itPoly(dagPath); !itPoly.isDone(); itPoly.next()) {
            if (itPoly.isUVReversed(&uvSet)) {
                result->push_back(createResultString(
                        dagPath, ResultType::Face,
                        static_cast<int>(itPoly.index())));
            }
        }
    }
}

} // unnamed namespace

UvChecker::UvChecker()
    : verbose(false),
    //   uvSet("map1"),
      minUVArea(0.000001),
      maxUvBorderDistance(0.0)
{
}

UvChecker::~UvChecker()
= default;

MSyntax UvChecker::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-v", "-verbose", MSyntax::kBoolean);
    syntax.addFlag("-c", "-check", MSyntax::kUnsigned);
    syntax.addFlag("-uva", "-uvArea", MSyntax::kDouble);
    syntax.addFlag("-us", "-uvSet", MSyntax::kString);
    syntax.addFlag("-muv", "-maxUvBorderDistance", MSyntax::kDouble);
    return syntax;
}

// MStringArray create_result_string(const MDagPath& path, const IndexArray& indices)
// {
//     MString full_path = path.fullPathName();
//     std::vector<MString> result;
//     result.reserve(indices.size());

//     for (auto index : indices) {
//         result.emplace_back(full_path + ".map[" + index + "]");
//     }
//     return { &result[0], static_cast<unsigned int>(indices.size()) };
// }

MStatus UvChecker::doIt(const MArgList& args)
{
    MStatus status;

    MSelectionList sel;

    MArgDatabase argData(syntax(), args);

    status = argData.getCommandArgument(0, sel);
    if (status != MS::kSuccess) {
        MGlobal::displayError("You have to provide an object path");
        return MStatus::kFailure;
    }

    MDagPath path;
    sel.getDagPath(0, path);
    MFnMesh mesh(path);
    // status = path.extendToShape();
    // CHECK_MSTATUS_AND_RETURN_IT(status)

    // Check if mesh or not
    // if (path.apiType() != MFn::kMesh) {
    //     MGlobal::displayError("Selected object is not mesh.");
    //     return MStatus::kFailure;
    // }

    // argument parsing
    UVCheckType check_type;

    if (argData.isFlagSet("-check")) {
        unsigned int check_value;
        argData.getFlagArgument("-check", 0, check_value);

        check_type = static_cast<UVCheckType>(check_value);

        // TODO check if exceeds value
    } else {
        MGlobal::displayError("Check type required.");
        return MS::kFailure;
    }

    if (argData.isFlagSet("-verbose"))
        argData.getFlagArgument("-verbose", 0, verbose);

    if (argData.isFlagSet("-uvArea"))
        argData.getFlagArgument("-uvArea", 0, minUVArea);

    if (argData.isFlagSet("-uvSet"))
        argData.getFlagArgument("-uvSet", 0, uvSet);
    else
        // mesh.getCurrentUVSetName(uvSet);
        uvSet = "map1";

    if (argData.isFlagSet("-maxUvBorderDistance"))
        argData.getFlagArgument("-maxUvBorderDistance", 0, maxUvBorderDistance);

    if (verbose) {
        MString objectPath = "Selected mesh : " + path.fullPathName();
        MGlobal::displayInfo(objectPath);
    }

    std::vector<std::string> hierarchy;
    buildHierarchy(path, hierarchy);

    // Number of threads to use
    size_t numTasks = 8;

    // Split sub-vectors to pass to each thread
    std::vector<std::vector<std::string>> splitGroups;

    splitGroups.resize(numTasks);
    size_t n = hierarchy.size() / numTasks + 1;
    size_t idCounter = 0;
    for (size_t groupID = 0; groupID < numTasks; groupID++) {
        splitGroups[groupID].reserve(n);
    }

    for (size_t a = 0; a < numTasks; a++) {
        for (size_t b = 0; b < n; b++) {
            if (idCounter == hierarchy.size()) {
                break;
            }
            splitGroups[a].emplace_back(hierarchy[idCounter]);
            idCounter++;
        }
    }

    std::vector<std::thread> threads;
    ResultStringArray result;


    if (check_type == UVCheckType::UDIM) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findUdimIntersections, &splitGroups[i], &result, uvSet, maxUvBorderDistance));
        }
    } else if(check_type == UVCheckType::HAS_UVS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findNoUvFaces, &splitGroups[i], &result, uvSet));
        }
    } else if (check_type == UVCheckType::ZERO_AREA) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findZeroUvFaces, &splitGroups[i], &result, uvSet, minUVArea));
        }
    } else if (check_type == UVCheckType::UN_ASSIGNED_UVS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(hasUnassignedUVs, &splitGroups[i], &result, uvSet));
        }
    } else if (check_type == UVCheckType::NEGATIVE_SPACE_UVS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findNegativeSpaceUVs, &splitGroups[i], &result, uvSet));
        }
    } else if (check_type == UVCheckType::CONCAVE_UVS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findConcaveUVs, &splitGroups[i], &result, uvSet));
        }
    } else if (check_type == UVCheckType::REVERSED_UVS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findReversedUVs, &splitGroups[i], &result, uvSet));
        }
    } else {
        MGlobal::displayError("Invalid check number");
        return MS::kFailure;
    }
    
    for (auto& t : threads) {
        t.join();
    }

    MStringArray finalResult;

    for (std::string& resultPath : result.data) {
        finalResult.append(resultPath.c_str());
    }

    setResult(finalResult);

    return redoIt();
}

MStatus UvChecker::redoIt()
{
    return MS::kSuccess;
}

MStatus UvChecker::undoIt()
{
    return MS::kSuccess;
}

bool UvChecker::isUndoable() const
{
    return false;
}

void* UvChecker::creator()
{
    return new UvChecker;
}