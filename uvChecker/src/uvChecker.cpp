#include "uvChecker.hpp"
#include <algorithm>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MFloatArray.h>
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <unordered_set>

using IndexArray = UvChecker::IndexArray;

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

MStringArray create_result_string(const MDagPath &path, const IndexArray &indices, ResultType type) {
    MString full_path = path.fullPathName();
    std::vector<MString> result;
    result.reserve(indices.size());

    for (auto index : indices) {
        switch (type) {
            case ResultType::Face: {
                result.emplace_back(full_path + ".f[" + index + "]");
                break;
            }
            case ResultType::Vertex: {
                result.emplace_back(full_path + ".vtx[" + index + "]");
                break;
            }

            case ResultType::Edge: {
                result.emplace_back(full_path + ".e[" + index + "]");
                break;
            }
            case ResultType::UV: {
                result.emplace_back(full_path + ".map[" + index + "]");
                break;
            }
        }
    }
    return {&result[0], static_cast<unsigned int>(indices.size())};
}

} // unnamed namespace

UvChecker::UvChecker()
    : verbose(false),
      uvSet("map1"),
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

MStringArray create_result_string(const MDagPath& path, const IndexArray& indices)
{
    MString full_path = path.fullPathName();
    std::vector<MString> result;
    result.reserve(indices.size());

    for (auto index : indices) {
        result.emplace_back(full_path + ".map[" + index + "]");
    }
    return { &result[0], static_cast<unsigned int>(indices.size()) };
}

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
    status = path.extendToShape();
    CHECK_MSTATUS_AND_RETURN_IT(status)

    // Check if mesh or not
    if (path.apiType() != MFn::kMesh) {
        MGlobal::displayError("Selected object is not mesh.");
        return MStatus::kFailure;
    }

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
        mesh.getCurrentUVSetName(uvSet);

    if (argData.isFlagSet("-maxUvBorderDistance"))
        argData.getFlagArgument("-maxUvBorderDistance", 0, maxUvBorderDistance);

    if (verbose) {
        MString objectPath = "Selected mesh : " + path.fullPathName();
        MGlobal::displayInfo(objectPath);
    }

    IndexArray indices;

    switch (check_type) {
    case UVCheckType::UDIM:
        if (verbose)
            MGlobal::displayInfo("Checking UDIM borders");
        indices = findUdimIntersections(mesh);
        setResult(create_result_string(path, indices, ResultType::UV));
        break;
    case UVCheckType::HAS_UVS:
        if (verbose)
            MGlobal::displayInfo("Checking Non UVed faces");
        indices = findNoUvFaces(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
        break;
    case UVCheckType::ZERO_AREA:
        if (verbose)
            MGlobal::displayInfo("Checking Zero UV faces");
        indices = findZeroUvFaces(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
        break;
    case UVCheckType::UN_ASSIGNED_UVS:
        if (verbose)
            MGlobal::displayInfo("Checking UnassignedUVs");
        bool result;
        result = hasUnassignedUVs(mesh);
        MPxCommand::setResult(result);
        break;
    case UVCheckType::NEGATIVE_SPACE_UVS:
        if (verbose)
            MGlobal::displayInfo("Checking UVs in negative space");
        indices = findNegativeSpaceUVs(mesh);
        setResult(create_result_string(path, indices, ResultType::UV));
        break;
    case UVCheckType::CONCAVE_UVS:
        if (verbose)
            MGlobal::displayInfo("Checking concave UVs");
        indices = findConcaveUVs(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
        break;
    case UVCheckType::REVERSED_UVS:
        if (verbose)
            MGlobal::displayInfo("Checking Reversed UVs");
        indices = findReversedUVs(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
        break;
    default:
        MGlobal::displayError("Invalid check number");
        return MS::kFailure;
    }

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

IndexArray UvChecker::findUdimIntersections(const MFnMesh& fnMesh)
{
    IndexArray indices;

    std::unordered_set<int> indexSet;

    MDagPath path;
    fnMesh.getPath(path);

    for (MItMeshPolygon mItPoly(path); !mItPoly.isDone(); mItPoly.next()) {

        int vCount = static_cast<int>(mItPoly.polygonVertexCount());
        int currentUVindex;
        int nextUVindex;
        float u1, v1, u2, v2;

        for (int i = 0; i < vCount; i++) {
            mItPoly.getUVIndex(i, currentUVindex, &uvSet);

            if (i == vCount - 1) {
                mItPoly.getUVIndex(0, nextUVindex, &uvSet);
            } else {
                mItPoly.getUVIndex(i + 1, nextUVindex, &uvSet);
            }

            fnMesh.getUV(currentUVindex, u1, v1, &uvSet);
            fnMesh.getUV(nextUVindex, u2, v2, &uvSet);

            if (floor(u1) == floor(u2) && floor(v1) == floor(v2)) {
            } else if ((maxUvBorderDistance == 0.0)
                || ((fabs(rint(u1) - fabs(u1)) > maxUvBorderDistance)
                    && (fabs(rint(v1) - fabs(v1)) > maxUvBorderDistance)
                    && (fabs(rint(u2) - fabs(u2)) > maxUvBorderDistance)
                    && (fabs(rint(v2) - fabs(v2)) > maxUvBorderDistance))) {
                indexSet.insert(currentUVindex);
                indexSet.insert(nextUVindex);
            }
        }
    }

    for (int indexSetIter : indexSet) {
        indices.emplace_back(indexSetIter);
    }
    return indices;
}

IndexArray UvChecker::findNoUvFaces(const MFnMesh& mesh)
{
    IndexArray indices;
    bool hasUVs;
    MDagPath path;
    mesh.getPath(path);
    for (MItMeshPolygon itPoly(path); !itPoly.isDone(); itPoly.next()) {
        hasUVs = itPoly.hasUVs(uvSet);
        if (!hasUVs) {
            indices.emplace_back(itPoly.index());
        }
    }
    return indices;
}

IndexArray UvChecker::findZeroUvFaces(const MFnMesh& mesh)
{
    double area;
    bool hasUVs;
    IndexArray indices;
    MDagPath path;
    mesh.getPath(path);
    for (MItMeshPolygon itPoly(path); !itPoly.isDone(); itPoly.next()) {
        hasUVs = itPoly.hasUVs(uvSet);
        if (!hasUVs) {
        } else {
            itPoly.getUVArea(area, &uvSet);
            if (area < minUVArea) {
                indices.emplace_back(itPoly.index());
            }
        }
    }
    return indices;
}

bool UvChecker::hasUnassignedUVs(const MFnMesh& mesh)
{
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
        return true;
    } else {
        return false;
    }
}

IndexArray UvChecker::findNegativeSpaceUVs(const MFnMesh& mesh)
{
    IndexArray indices;
    MFloatArray uArray, vArray;
    mesh.getUVs(uArray, vArray, &uvSet);

    int numUVs = mesh.numUVs(uvSet);

    for (int i = 0; i < numUVs; i++) {
        float u = uArray[static_cast<unsigned int>(i)];
        if (u < 0.0) {
            indices.push_back(i);
            continue;
        }
        float v = vArray[static_cast<unsigned int>(i)];
        if (v < 0.0) {
            indices.push_back(i);
            continue;
        }
    }

    // Remove duplicate elements
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    return indices;
}

// Find concave UVs by iterating each polygon and its internal triangles.
// If area of triangle is 0 or negative, triangle vertex order is reversed
// and the face is hence concave UVed polygon.
IndexArray UvChecker::findConcaveUVs(const MFnMesh& mesh)
{
    IndexArray indices;
    MDagPath path;
    mesh.getPath(path);
    MPointArray points;
    MIntArray vertexList;

    for (MItMeshPolygon itPoly(path); !itPoly.isDone(); itPoly.next()) {
        unsigned int polygonIndex = itPoly.index();
        itPoly.getTriangles(points, vertexList);
        int numVerts = static_cast<int>(itPoly.polygonVertexCount());

        int p1, p2, p3;

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

            itPoly.getUV(p1, uv1);
            itPoly.getUV(p2, uv2);
            itPoly.getUV(p3, uv3);

            float S = getTriangleArea(uv1[0], uv1[1], uv2[0], uv2[1], uv3[0], uv3[1]);

            if (S <= 0.00000000001) {
                indices.push_back(static_cast<int>(polygonIndex));
            }
        }
    }
    return indices;
}

IndexArray UvChecker::findReversedUVs(const MFnMesh& mesh) {
    IndexArray indices;
    MDagPath path;
    mesh.getPath(path);
    for (MItMeshPolygon itPoly(path); !itPoly.isDone(); itPoly.next()) {
        if (itPoly.isUVReversed()) {
            indices.push_back(itPoly.index());
        }
    }
    return indices;
}