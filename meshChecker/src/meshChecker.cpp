#include "meshChecker.hpp"
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

#include <future>
#include <cmath>
#include <string>
#include <thread>

using IndexArray = MeshChecker::IndexArray;

namespace {

enum class ResultType {
    Face,
    Vertex,
    Edge,
    UV
};

MStringArray create_result_string(const MDagPath& path, const IndexArray& indices, ResultType type)
{
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
    return { &result[0], static_cast<unsigned int>(indices.size()) };
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

void findTrianglesMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findNgonsMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findNonManifoldEdgesMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findLaminaFacesMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findBiValentFacesMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findZeroAreaFacesMT(std::vector<std::string>* paths, ResultStringArray* result, double maxFaceArea)
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

void findMeshBorderEdgesMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findCreaseEdgesMT(std::vector<std::string>* paths, ResultStringArray* result)
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

void findZeroLengthEdgesMT(std::vector<std::string>* paths, ResultStringArray* result, double minEdgeLength)
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

void hasVertexPntsAttrMT(std::vector<std::string>* paths, ResultStringArray* result) {
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

        std::string tempPath = dagPath.fullPathName().asChar();

        while (true) {
            outputHandle = arrayDataHandle.outputValue();

            float3& xyz = outputHandle.asFloat3();
            if (xyz) {
                if (xyz[0] != 0.0) {
                    result->push_back(tempPath);
                    pntsArray.destructHandle(dataHandle);
                    break;
                }
                if (xyz[1] != 0.0) {
                    result->push_back(tempPath);
                    pntsArray.destructHandle(dataHandle);
                    break;
                }
                if (xyz[2] != 0.0) {
                    result->push_back(tempPath);
                    pntsArray.destructHandle(dataHandle);
                    break;
                }
            }
            status = arrayDataHandle.next();
            if (status != MS::kSuccess) {
                break;
            }
        }
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

IndexArray MeshChecker::findTriangles(const MFnMesh& mesh)
{
    auto num_polygons = mesh.numPolygons();

    IndexArray indices;
    indices.reserve(static_cast<size_t>(num_polygons));

    for (Index poly_index {}; poly_index < num_polygons; ++poly_index) {
        if (mesh.polygonVertexCount(poly_index) == 3) {
            indices.push_back(poly_index);
        }
    }
    return indices;
}

IndexArray MeshChecker::findNgons(const MFnMesh& mesh)
{
    auto num_polygons = mesh.numPolygons();

    IndexArray indices;
    indices.reserve(static_cast<size_t>(num_polygons));

    for (Index poly_index {}; poly_index < num_polygons; ++poly_index) {
        if (mesh.polygonVertexCount(poly_index) >= 5) {
            indices.push_back(poly_index);
        }
    }
    return indices;
}

IndexArray MeshChecker::findNonManifoldEdges(const MFnMesh& mesh)
{
    MDagPath path;
    mesh.getPath(path);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(mesh.numEdges()));

    for (MItMeshEdge edge_it(path); !edge_it.isDone(); edge_it.next()) {
        int face_count;
        edge_it.numConnectedFaces(face_count);
        if (face_count > 2) {
            indices.push_back(edge_it.index());
        }
    }
    return indices;
}

IndexArray MeshChecker::findLaminaFaces(const MFnMesh& mesh)
{
    MDagPath path;
    mesh.getPath(path);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(mesh.numPolygons()));

    for (MItMeshPolygon poly_it(path); !poly_it.isDone(); poly_it.next()) {
        if (poly_it.isLamina()) {
            indices.push_back(static_cast<Index>(poly_it.index()));
        }
    }
    return indices;
}

IndexArray MeshChecker::findBiValentFaces(const MFnMesh& mesh)
{
    MDagPath path;
    mesh.getPath(path);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(mesh.numVertices()));

    MIntArray connectedFaces;
    MIntArray connectedEdges;

    for (MItMeshVertex vertex_it(path); !vertex_it.isDone(); vertex_it.next()) {
        vertex_it.getConnectedFaces(connectedFaces);
        vertex_it.getConnectedEdges(connectedEdges);

        if (connectedFaces.length() == 2 && connectedEdges.length() == 2) {
            indices.push_back(vertex_it.index());
        }
    }
    return indices;
}

IndexArray MeshChecker::findZeroAreaFaces(const MFnMesh& mesh, double maxFaceArea)
{
    MDagPath path;
    mesh.getPath(path);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(mesh.numPolygons()));

    for (MItMeshPolygon poly_it(path); !poly_it.isDone(); poly_it.next()) {
        double area;
        poly_it.getArea(area);
        if (area < maxFaceArea) {
            indices.push_back(static_cast<Index>(poly_it.index()));
        }
    }
    return indices;
}

IndexArray MeshChecker::findMeshBorderEdges(const MFnMesh& mesh)
{
    MDagPath path;
    mesh.getPath(path);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(mesh.numEdges()));

    for (MItMeshEdge edge_it(path); !edge_it.isDone(); edge_it.next()) {
        if (edge_it.onBoundary()) {
            indices.push_back(edge_it.index());
        }
    }
    return indices;
}

IndexArray MeshChecker::findCreaseEdges(const MFnMesh& mesh)
{
    MUintArray edgeIds;
    MDoubleArray creaseData;
    mesh.getCreaseEdges(edgeIds, creaseData);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(edgeIds.length()));

    for (unsigned int i {}; i < edgeIds.length(); i++) {
        if (creaseData[i] != 0) {
            indices.push_back(static_cast<Index>(edgeIds[i]));
        }
    }
    return indices;
}

IndexArray MeshChecker::findZeroLengthEdges(const MFnMesh& mesh, double minEdgeLength)
{
    MDagPath path;
    mesh.getPath(path);

    IndexArray indices;
    indices.reserve(static_cast<size_t>(mesh.numEdges()));

    for (MItMeshEdge edge_it(path); !edge_it.isDone(); edge_it.next()) {
        double length;
        edge_it.getLength(length);
        if (length < minEdgeLength) {
            indices.push_back(edge_it.index());
        }
    }
    return indices;
}

bool MeshChecker::hasVertexPntsAttr(const MFnMesh& mesh, bool fix)
{
    MDagPath path;
    mesh.getPath(path);

    MStatus status;

    path.extendToShape();
    MFnDagNode dagNode(path);
    MPlug pntsArray = mesh.findPlug("pnts", false);
    MDataHandle dataHandle = pntsArray.asMDataHandle();
    MArrayDataHandle arrayDataHandle(dataHandle);
    MDataHandle outputHandle;

    if (!fix) {
        // Check only.

        while (true) {
            outputHandle = arrayDataHandle.outputValue();

            float3& xyz = outputHandle.asFloat3();
            if (xyz) {
                if (xyz[0] != 0.0) {
                    pntsArray.destructHandle(dataHandle);
                    return true;
                }
                if (xyz[1] != 0.0) {
                    pntsArray.destructHandle(dataHandle);
                    return true;
                }
                if (xyz[2] != 0.0) {
                    pntsArray.destructHandle(dataHandle);
                    return true;
                }
            }
            status = arrayDataHandle.next();
            if (status != MS::kSuccess) {
                break;
            }
        }
    } else {
        // Do fix. Reset all vertices pnts attr to 0
        MObject pntx = dagNode.attribute("pntx");
        MObject pnty = dagNode.attribute("pnty");
        MObject pntz = dagNode.attribute("pntz");
        MDataHandle xHandle;
        MDataHandle yHandle;
        MDataHandle zHandle;

        while (true) {
            outputHandle = arrayDataHandle.outputValue();

            // outputHandle.set3Double(0.0, 0.0, 0.0);

            // setting 3 values at the same time kills maya in
            // some environments somehow. So here setting values separately
            xHandle = outputHandle.child(pntx);
            yHandle = outputHandle.child(pnty);
            zHandle = outputHandle.child(pntz);
            xHandle.setFloat(0.0);
            yHandle.setFloat(0.0);
            zHandle.setFloat(0.0);

            status = arrayDataHandle.next();
            if (status != MS::kSuccess) {
                break;
            }
        }
        pntsArray.setMDataHandle(dataHandle);
    }
    pntsArray.destructHandle(dataHandle);
    return false;
}

bool MeshChecker::isEmpty(const MFnMesh& mesh)
{
    int numVerts = mesh.numVertices();
    if (numVerts == 0) {
        return true;
    } else {
        return false;
    }
}

void MeshChecker::checkMT(const MDagPath& rootPath, MeshCheckType checkType)
{
    std::vector<std::string> hierarchy;
    buildHierarchy(rootPath, hierarchy);

    // Number of threads to use
    size_t numTasks = 8;

    // Split sub-vectors to pass to each thread
    std::vector<std::vector<std::string>> splitGroups;

    splitGroups.resize(numTasks);
    size_t n = hierarchy.size() / numTasks + 1;
    size_t idCounter = 0;
    for (int i=0; i<numTasks; i++) {
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

    if (checkType == MeshCheckType::TRIANGLES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findTrianglesMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::NGONS) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findNgonsMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::NON_MANIFOLD_EDGES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findNonManifoldEdgesMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::LAMINA_FACES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findLaminaFacesMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::BI_VALENT_FACES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findBiValentFacesMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::ZERO_AREA_FACES) {
        double maxFaceArea { 0.000001 };
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findZeroAreaFacesMT, &splitGroups[i], &result, maxFaceArea));
        }
    } else if (checkType == MeshCheckType::MESH_BORDER) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findMeshBorderEdgesMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::CREASE_EDGE) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findCreaseEdgesMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::ZERO_LENGTH_EDGES) {
        double minEdgeLength = 0.000001;
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(findZeroLengthEdgesMT, &splitGroups[i], &result, minEdgeLength));
        }
    } else if (checkType == MeshCheckType::UNFROZEN_VERTICES) {
        for (size_t i = 0; i < numTasks; i++) {
            threads.push_back(std::thread(hasVertexPntsAttrMT, &splitGroups[i], &result));
        }
    } else if (checkType == MeshCheckType::EMPTY_GEOMETRY) {
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

    if (argData.isFlagSet("-multiThreaded")) {
        if (argData.isFlagSet("-multiThreaded"))
            argData.getFlagArgument("-multiThreaded", 0, isMultiThreaded);
    } else {
    }

    // mesh construction
    MDagPath path;
    selection.getDagPath(0, path);

    if (!isMultiThreaded) {
        // Check if selected object is geometry
        status = path.extendToShape();
        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to extend to shape node. Not mesh");
            return MS::kFailure;
        }

        if (path.apiType() != MFn::kMesh) {
            MGlobal::displayError("MeshCheker works on meshes.");
            return MS::kFailure;
        }
    }

    MFnMesh mesh { path };

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

    if (isMultiThreaded) {
        checkMT(path, check_type);
        return MS::kSuccess;
    }

    // execute operation
    if (check_type == MeshCheckType::TRIANGLES) {
        auto indices = findTriangles(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
    } else if (check_type == MeshCheckType::NGONS) {
        auto indices = findNgons(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
    } else if (check_type == MeshCheckType::NON_MANIFOLD_EDGES) {
        auto indices = findNonManifoldEdges(mesh);
        setResult(create_result_string(path, indices, ResultType::Edge));
    } else if (check_type == MeshCheckType::LAMINA_FACES) {
        auto indices = findLaminaFaces(mesh);
        setResult(create_result_string(path, indices, ResultType::Face));
    } else if (check_type == MeshCheckType::BI_VALENT_FACES) {
        auto indices = findBiValentFaces(mesh);
        setResult(create_result_string(path, indices, ResultType::Vertex));
    } else if (check_type == MeshCheckType::ZERO_AREA_FACES) {
        double maxFaceArea { 0.000001 };
        if (argData.isFlagSet("-maxFaceArea"))
            argData.getFlagArgument("-maxFaceArea", 0, maxFaceArea);

        auto indices = findZeroAreaFaces(mesh, maxFaceArea);
        setResult(create_result_string(path, indices, ResultType::Face));
    } else if (check_type == MeshCheckType::MESH_BORDER) {
        auto indices = findMeshBorderEdges(mesh);
        setResult(create_result_string(path, indices, ResultType::Edge));
    } else if (check_type == MeshCheckType::CREASE_EDGE) {
        auto indices = findCreaseEdges(mesh);
        setResult(create_result_string(path, indices, ResultType::Edge));
    } else if (check_type == MeshCheckType::ZERO_LENGTH_EDGES) {
        double minEdgeLength = 0.000001;
        if (argData.isFlagSet("-minEdgeLength"))
            argData.getFlagArgument("-minEdgeLength", 0, minEdgeLength);

        auto indices = findZeroLengthEdges(mesh, minEdgeLength);
        setResult(create_result_string(path, indices, ResultType::Edge));
    } else if (check_type == MeshCheckType::UNFROZEN_VERTICES) {
        bool fix = false;
        if (argData.isFlagSet("-fix"))
            argData.getFlagArgument("-fix", 0, fix);
        setResult(hasVertexPntsAttr(mesh, fix));
    } else if (check_type == MeshCheckType::EMPTY_GEOMETRY) {
        setResult(isEmpty(mesh));
    } else if (check_type == MeshCheckType::TEST) {
        ;
    } else {
        MGlobal::displayError("Invalid check number");
        return MS::kFailure;
    }

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
    syntax.addFlag("-mt", "-multiThreaded", MSyntax::kBoolean);
    return syntax;
}
