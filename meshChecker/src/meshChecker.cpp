#include "meshChecker.hpp"

#include <cstddef>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MDagPath.h>
#include <maya/MDataHandle.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
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

#include <cmath>
#include <string>
#include <thread>

namespace {

enum class ResultType {
    Face,
    Vertex,
    Edge,
    UV
};

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

void findTriangles(std::vector<std::string>* paths, ResultStringArray* result)
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
        int numPoly = mesh.numPolygons();

        for (int j = 0; j < numPoly; j++) {
            if (mesh.polygonVertexCount(j) == 3) {
                result->push_back(createResultString(dagPath, ResultType::Face, j));
            }
        }
    }
}

void findNgons(std::vector<std::string>* paths, ResultStringArray* result)
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
        int numPoly = mesh.numPolygons();

        for (int i = 0; i < numPoly; i++) {
            if (mesh.polygonVertexCount(i) >= 5) {
                result->push_back(createResultString(dagPath, ResultType::Face, i));
            }
        }
    }
}

void findNonManifoldEdges(std::vector<std::string>* paths, ResultStringArray* result)
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

        for (MItMeshEdge edgeIter(dagPath); !edgeIter.isDone(); edgeIter.next()) {
            int face_count;
            edgeIter.numConnectedFaces(face_count);
            if (face_count > 2) {
                result->push_back(createResultString(dagPath, ResultType::Edge, edgeIter.index()));
            }
        }
    }
}

void findLaminaFaces(std::vector<std::string>* paths, ResultStringArray* result)
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

        for (MItMeshPolygon polyIter(dagPath); !polyIter.isDone(); polyIter.next()) {
            if (polyIter.isLamina()) {
                result->push_back(
                    createResultString(
                        dagPath,
                        ResultType::Face,
                        static_cast<int>(polyIter.index())));
            }
        }
    }
}

void findBiValentFaces(std::vector<std::string>* paths, ResultStringArray* result)
{
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

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
                result->push_back(createResultString(dagPath, ResultType::Vertex, vtxIter.index()));
            }
        }
    }
}

void findZeroAreaFaces(std::vector<std::string>* paths, ResultStringArray* result, double maxFaceArea)
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

        for (MItMeshPolygon polyIter(dagPath); !polyIter.isDone(); polyIter.next()) {
            double area;
            polyIter.getArea(area);
            if (area < maxFaceArea) {
                result->push_back(
                    createResultString(
                        dagPath,
                        ResultType::Face,
                        static_cast<int>(polyIter.index())));
            }
        }
    }
}

void findMeshBorderEdges(std::vector<std::string>* paths, ResultStringArray* result)
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

        for (MItMeshEdge edgeIter(dagPath); !edgeIter.isDone(); edgeIter.next()) {
            if (edgeIter.onBoundary()) {
                result->push_back(createResultString(dagPath, ResultType::Edge, edgeIter.index()));
            }
        }
    }
}

void findCreaseEdges(std::vector<std::string>* paths, ResultStringArray* result)
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

        MUintArray edgeIds;
        MDoubleArray creaseData;
        mesh.getCreaseEdges(edgeIds, creaseData);

        unsigned int edgeIdLength = edgeIds.length();

        for (unsigned int j = 0; j < edgeIdLength; j++) {
            result->push_back(
                createResultString(dagPath, ResultType::Edge, static_cast<int>(edgeIds[j])));
        }
    }
}

void findZeroLengthEdges(std::vector<std::string>* paths, ResultStringArray* result, double minEdgeLength)
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
        for (MItMeshEdge edgeIter(dagPath); !edgeIter.isDone(); edgeIter.next()) {
            double length;
            edgeIter.getLength(length);
            if (length < minEdgeLength) {
                result->push_back(
                    createResultString(
                        dagPath,
                        ResultType::Edge,
                        static_cast<int>(edgeIter.index())));
            }
        }
    }
}

void hasVertexPntsAttr(std::vector<std::string>* paths, ResultStringArray* result) {
    MSelectionList list;

    for (auto& p : *paths) {
        MString mPath(p.c_str());
        list.add(mPath);
    }

    unsigned int length = list.length();

    MDagPath dagPath;

    MStatus status;

    for (unsigned int i=0; i < length; i++) {
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
                result->push_back(dagPath.fullPathName().asChar());
                break;
            }           
            if (xyz[1] != 0.0) {
                result->push_back(dagPath.fullPathName().asChar());
                break;
            }
            if (xyz[2] != 0.0) {
                result->push_back(dagPath.fullPathName().asChar());
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
}

void isEmptyGeometry(std::vector<std::string>* paths, ResultStringArray* result)
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
        int numVerts = mesh.numVertices();
        if (numVerts == 0) {
            result->push_back(dagPath.fullPathName().asChar());
        }
    }
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
    for (size_t i=0; i<numTasks; i++) {
        splitGroups[i].reserve(n);
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

    if (check_type == MeshCheckType::TRIANGLES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findTriangles, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::NGONS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findNgons, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::NON_MANIFOLD_EDGES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findNonManifoldEdges, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::LAMINA_FACES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findLaminaFaces, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::BI_VALENT_FACES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findBiValentFaces, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::ZERO_AREA_FACES) {
        double maxFaceArea { 0.000001 };
        if (argData.isFlagSet("-maxFaceArea"))
            argData.getFlagArgument("-maxFaceArea", 0, maxFaceArea);
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findZeroAreaFaces, &splitGroups[i], &result, maxFaceArea));
        }
    } else if (check_type == MeshCheckType::MESH_BORDER) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findMeshBorderEdges, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::CREASE_EDGE) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findCreaseEdges, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::ZERO_LENGTH_EDGES) {
        double minEdgeLength = 0.000001;
        if (argData.isFlagSet("-minEdgeLength"))
            argData.getFlagArgument("-minEdgeLength", 0, minEdgeLength);
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findZeroLengthEdges, &splitGroups[i], &result, minEdgeLength));
        }
    } else if (check_type == MeshCheckType::UNFROZEN_VERTICES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(hasVertexPntsAttr, &splitGroups[i], &result));
        }
    } else if (check_type == MeshCheckType::EMPTY_GEOMETRY) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(isEmptyGeometry, &splitGroups[i], &result));
        }
    } else {
        std::cout << "not supported yet" << std::endl;
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
