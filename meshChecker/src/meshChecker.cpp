#include "meshChecker.hpp"
#include "../../include/ThreadPool.hpp"
#include "../../include/utils.hpp"
#include "maya/MApiNamespace.h"

#include <cstddef>
#include <maya/MObject.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MDagPath.h>
#include <maya/MDataHandle.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MUintArray.h>
#include <maya/MPlugArray.h>

#include <cmath>
#include <string>
#include <thread>
#include <algorithm>

static const char* const pluginCommandName = "checkMesh";
static const char* const pluginVersion = "2.1.4";
static const char* const pluginAuthor = "Michi Inoue";

namespace {

std::vector<std::string> findTriangles(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        int numPoly = mesh.numPolygons();

        for (int j = 0; j < numPoly; j++) {
            if (mesh.polygonVertexCount(j) == 3) {
                createResultString(dagPath, ResultType::Face, j, errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findNgons(std::vector<std::string>* paths)
{

    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        int numPoly = mesh.numPolygons();

        for (int i = 0; i < numPoly; i++) {
            if (mesh.polygonVertexCount(i) >= 5) {
                createResultString(dagPath, ResultType::Face, i, errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findNonManifoldEdges(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);

        for (MItMeshEdge edgeIter(dagPath); !edgeIter.isDone(); edgeIter.next()) {
            int face_count;
            edgeIter.numConnectedFaces(face_count);
            if (face_count > 2) {
                createResultString(dagPath, ResultType::Edge, edgeIter.index(), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findLaminaFaces(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);

        for (MItMeshPolygon polyIter(dagPath); !polyIter.isDone(); polyIter.next()) {
            if (polyIter.isLamina()) {
                createResultString(dagPath, ResultType::Face, static_cast<int>(polyIter.index()), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findBiValentFaces(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    MIntArray connectedFaces;
    MIntArray connectedEdges;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);

        for (MItMeshVertex vtxIter(dagPath); !vtxIter.isDone(); vtxIter.next()) {
            vtxIter.getConnectedFaces(connectedFaces);
            vtxIter.getConnectedEdges(connectedEdges);

            if (connectedFaces.length() == 2 && connectedEdges.length() == 2) {
                createResultString(dagPath, ResultType::Vertex, vtxIter.index(), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findZeroAreaFaces(std::vector<std::string>* paths, double maxFaceArea)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);

        for (MItMeshPolygon polyIter(dagPath); !polyIter.isDone(); polyIter.next()) {
            double area;
            polyIter.getArea(area);
            if (area < maxFaceArea) {
                createResultString(dagPath, ResultType::Face, static_cast<int>(polyIter.index()), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findMeshBorderEdges(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);

        for (MItMeshEdge edgeIter(dagPath); !edgeIter.isDone(); edgeIter.next()) {
            if (edgeIter.onBoundary()) {
                createResultString(dagPath, ResultType::Edge, edgeIter.index(), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findCreaseEdges(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    std::vector<std::string> result;
    std::string errorPath;

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);

        MUintArray edgeIds;
        MDoubleArray creaseData;
        mesh.getCreaseEdges(edgeIds, creaseData);

        unsigned int edgeIdLength = edgeIds.length();

        for (unsigned int j = 0; j < edgeIdLength; j++) {
            createResultString(dagPath, ResultType::Edge, static_cast<int>(edgeIds[j]), errorPath);
            result.push_back(errorPath);
        }
    }
    return result;
}

std::vector<std::string> findZeroLengthEdges(std::vector<std::string>* paths, double minEdgeLength)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        for (MItMeshEdge edgeIter(dagPath); !edgeIter.isDone(); edgeIter.next()) {
            double length;
            edgeIter.getLength(length);
            if (length < minEdgeLength) {
                createResultString(dagPath, ResultType::Edge, static_cast<int>(edgeIter.index()), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> hasVertexPntsAttr(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;

    unsigned int length = list.length();

    MDagPath dagPath;

    MStatus status;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        dagPath.extendToShape();
        MFnDagNode dagNode(dagPath);
        MFnMesh mesh(dagPath);
        MPlug pntsArray = mesh.findPlug("pnts", false);
        MDataHandle dataHandle = pntsArray.asMDataHandle();
        MArrayDataHandle arrayDataHandle(dataHandle);
        MDataHandle outputHandle;

        unsigned int numElements = arrayDataHandle.elementCount();

        if (numElements == 0) {
            continue;
        }

        while (true) {
            outputHandle = arrayDataHandle.outputValue();

            const float3& xyz = outputHandle.asFloat3();

            if (xyz[0] != 0.0) {
                result.push_back(dagPath.fullPathName().asChar());
                break;
            }
            if (xyz[1] != 0.0) {
                result.push_back(dagPath.fullPathName().asChar());
                break;
            }
            if (xyz[2] != 0.0) {
                result.push_back(dagPath.fullPathName().asChar());
                break;
            }

            // end of iterator
            status = arrayDataHandle.next();
            if (status != MS::kSuccess) {
                break;
            }
        }
        pntsArray.destructHandle(dataHandle);
    }
    return result;
}

std::vector<std::string> isEmptyGeometry(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;

    unsigned int length = list.length();
    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        MFnMesh mesh(dagPath);
        int numVerts = mesh.numVertices();
        if (numVerts == 0) {
            result.push_back(dagPath.fullPathName().asChar());
        }
    }
    return result;
}

std::vector<std::string> findUnusedVertices(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();
    MDagPath dagPath;

    for (unsigned int i = 0; i < length; i++) {
        list.getDagPath(i, dagPath);
        int edgeCount;

        for (MItMeshVertex vtxIter(dagPath); !vtxIter.isDone(); vtxIter.next()) {
            vtxIter.numConnectedEdges(edgeCount);

            if (edgeCount == 0) {
                createResultString(dagPath, ResultType::Vertex, vtxIter.index(), errorPath);
                result.push_back(errorPath);
            }
        }
    }
    return result;
}

std::vector<std::string> findInstances(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    std::vector<std::string> result;
    std::string errorPath;

    unsigned int length = list.length();
    MDagPath dagPath;
    MFnDagNode fnDag;

    for (unsigned int i=0; i < length; i++) {
        list.getDagPath(i, dagPath);
        fnDag.setObject(dagPath);
        if (fnDag.isInstanced()) {
            MObject mObj = fnDag.parent(0);
            MFnDagNode dataParent(mObj);
            MString instanceSource = dataParent.fullPathName();
            dagPath.pop(1);
            MString hierarchyParent = dagPath.fullPathName();
            if (hierarchyParent != instanceSource) {
                result.push_back(dagPath.fullPathName().asChar());
            }
        }
    }

    return result;
}

std::vector<std::string> findConnections(std::vector<std::string>* paths)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }
    std::vector<std::string> CON_LIST = {
        "translateX",
        "translateY",
        "translateZ",
        "rotateX",
        "rotateY",
        "rotateZ",
        "rotateOrder",
        "parentInverseMatrix",
        "rotatePivot",
        "rotatePivotTranslate"
    };

    std::vector<std::string> result;

    unsigned int length = list.length();

    MDagPath dagPath;

    for (unsigned int i=0; i < length; i++) {

        MPlugArray plugs;

        list.getDagPath(i, dagPath);
        dagPath.pop(1);
        MObject mObj = dagPath.node();
        MFnDependencyNode fnDep(mObj);
        fnDep.getConnections(plugs);
        unsigned int numPlugs = plugs.length();
        for (unsigned int j=0; j<numPlugs; j++) {
            MPlug p = plugs[j];
            MString cn = p.partialName(false, false, false, false, false, true);
            if (std::find(CON_LIST.begin(), CON_LIST.end(), cn.asChar()) != CON_LIST.end()) {
                result.push_back(dagPath.fullPathName().asChar());
                break;
            }
        }
    }
    return result;
}

} // namespace

MeshChecker::MeshChecker()
    : MPxCommand()
{
}

MStatus MeshChecker::doIt(const MArgList& args)
{

    MStatus status;
    MArgDatabase argData(syntax(), args);

    // if argument is not provided use selection list
    MSelectionList selection;
    if (args.length() == 0) {
        MGlobal::getActiveSelectionList(selection);
    } else {
        status = argData.getCommandArgument(0, selection);
        CHECK_MSTATUS_AND_RETURN_IT(status)
        if (status != MS::kSuccess) {
            return MStatus::kFailure;
        }
    }

    // mesh construction
    MDagPath path;
    selection.getDagPath(0, path);

    // argument parsing
    MeshCheckType check_type;

    if (argData.isFlagSet("-check")) {
        unsigned int check_value;
        argData.getFlagArgument("-check", 0, check_value);

        check_type = static_cast<MeshCheckType>(check_value);

        // TODO check if exeds value
    } else {
        MGlobal::displayError("Check type required.");
        return MS::kFailure;
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

    ThreadPool pool(8);
    std::vector<std::future<std::vector<std::string>>> results;

    double maxFaceArea { 0.000001 };
    if (argData.isFlagSet("-maxFaceArea"))
        argData.getFlagArgument("-maxFaceArea", 0, maxFaceArea);

    double minEdgeLength = 0.000001;
    if (argData.isFlagSet("-minEdgeLength"))
        argData.getFlagArgument("-minEdgeLength", 0, minEdgeLength);

    for (size_t i = 0; i < numTasks; i++) {
        if (check_type == MeshCheckType::TRIANGLES) {
            results.push_back(pool.enqueue(findTriangles, &splitGroups[i]));
        } else if (check_type == MeshCheckType::NGONS) {
            results.push_back(pool.enqueue(findNgons, &splitGroups[i]));
        } else if (check_type == MeshCheckType::NON_MANIFOLD_EDGES) {
            results.push_back(pool.enqueue(findNonManifoldEdges, &splitGroups[i]));
        } else if (check_type == MeshCheckType::LAMINA_FACES) {
            results.push_back(pool.enqueue(findLaminaFaces, &splitGroups[i]));
        } else if (check_type == MeshCheckType::BI_VALENT_FACES) {
            results.push_back(pool.enqueue(findBiValentFaces, &splitGroups[i]));
        } else if (check_type == MeshCheckType::ZERO_AREA_FACES) {
            results.push_back(pool.enqueue(findZeroAreaFaces, &splitGroups[i], maxFaceArea));
        } else if (check_type == MeshCheckType::MESH_BORDER) {
            results.push_back(pool.enqueue(findMeshBorderEdges, &splitGroups[i]));
        } else if (check_type == MeshCheckType::CREASE_EDGE) {
            results.push_back(pool.enqueue(findCreaseEdges, &splitGroups[i]));
        } else if (check_type == MeshCheckType::ZERO_LENGTH_EDGES) {
            results.push_back(pool.enqueue(findZeroLengthEdges, &splitGroups[i], minEdgeLength));
        } else if (check_type == MeshCheckType::UNFROZEN_VERTICES) {
            results.push_back(pool.enqueue(hasVertexPntsAttr, &splitGroups[i]));
        } else if (check_type == MeshCheckType::EMPTY_GEOMETRY) {
            results.push_back(pool.enqueue(isEmptyGeometry, &splitGroups[i]));
        } else if (check_type == MeshCheckType::UNUSED_VERTICES) {
            results.push_back(pool.enqueue(findUnusedVertices, &splitGroups[i]));
        } else if (check_type == MeshCheckType::INSTANCE) {
            results.push_back(pool.enqueue(findInstances, &splitGroups[i]));
        } else if (check_type == MeshCheckType::CONNECTIONS) {
            results.push_back(pool.enqueue(findConnections, &splitGroups[i]));
        } else {
            MGlobal::displayError("Invalid check number");
            return MS::kFailure;
        }
    }

    std::vector<std::string> intermediateResult;

    for (auto&& result : results) {
        std::vector<std::string> temp = result.get();
        for (auto& r : temp) {
            intermediateResult.push_back(r);
        }
    }

    MStringArray outputResult;

    for (std::string& path : intermediateResult) {
        outputResult.append(path.c_str());
    }

    setResult(outputResult);

    return redoIt();
}

MStatus MeshChecker::redoIt()
{
    return MS::kSuccess;
}

MStatus MeshChecker::undoIt()
{
    return MS::kSuccess;
}

bool MeshChecker::isUndoable() const
{
    return false;
}

void* MeshChecker::creator()
{
    return new MeshChecker;
}

MSyntax MeshChecker::newSyntax()
{
    MSyntax syntax;
    syntax.addArg(MSyntax::kString);
    syntax.addFlag("-c", "-check", MSyntax::kUnsigned);
    syntax.addFlag("-mfa", "-maxFaceArea", MSyntax::kDouble);
    syntax.addFlag("-mel", "-minEdgeLength", MSyntax::kDouble);
    syntax.addFlag("-fix", "-doFix", MSyntax::kBoolean);
    return syntax;
}

MStatus initializePlugin(MObject mObj)
{
    MStatus status;

    std::string version_str(pluginVersion);
    std::string compile_date_str(__DATE__);
    std::string compile_time_str(__TIME__);
    std::string version(version_str + " / " + compile_date_str + " / " + compile_time_str);

    MFnPlugin fnPlugin(mObj, pluginAuthor, version.c_str(), "Any");

    status = fnPlugin.registerCommand(pluginCommandName, MeshChecker::creator, MeshChecker::newSyntax);
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