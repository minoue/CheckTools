#include "meshChecker.h"
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>
#include <maya/MUintArray.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>

MeshChecker::MeshChecker() {
}

MeshChecker::~MeshChecker() {
}

MStatus MeshChecker::findTriangles() {
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() == 3)
            indexArray.append(mItPoly.index());
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findNgons() {
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.polygonVertexCount() >= 5)
            indexArray.append(mItPoly.index());
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findNonManifoldEdges() {
    MStatus status;
    int faceCount;
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        mItEdge.numConnectedFaces(faceCount);
        if (faceCount > 2) {
            indexArray.append(mItEdge.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findLaminaFaces() {
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        if (mItPoly.isLamina() == true)
            indexArray.append(mItPoly.index());
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findBiValentFaces() {
    MIntArray connectedFaces;
    MIntArray connectedEdges;
    for (MItMeshVertex mItVert(mDagPath); !mItVert.isDone(); mItVert.next()) {
        mItVert.getConnectedFaces(connectedFaces);
        mItVert.getConnectedEdges(connectedEdges);
        int numFaces = connectedFaces.length();
        int numEdges = connectedEdges.length();
        if (numFaces == 2 && numEdges == 2) {
            indexArray.append(mItVert.index());
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findZeroAreaFaces(double &maxFaceArea) {
    double area;
    for (MItMeshPolygon mItPoly(mDagPath); !mItPoly.isDone(); mItPoly.next()) {
        mItPoly.getArea(area);
        if (area < maxFaceArea)
            indexArray.append(mItPoly.index());
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findMeshBorderEdges() {
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        bool isBorder = mItEdge.onBoundary();
        if (isBorder)
            indexArray.append(mItEdge.index());
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findCreaseEDges() {
    MFnMesh fnMesh(mDagPath);
    MUintArray edgeIds;
    MDoubleArray creaseData;
    fnMesh.getCreaseEdges(edgeIds, creaseData);

    if (edgeIds.length() != 0) {
        for (unsigned int i = 0; i < edgeIds.length(); i++) {
            if (creaseData[i] == 0)
                continue;
            int edgeId = (int) edgeIds[i];
            indexArray.append(edgeId);
        }
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findZeroLengthEdges() {
    double length;
    for (MItMeshEdge mItEdge(mDagPath); !mItEdge.isDone(); mItEdge.next()) {
        mItEdge.getLength(length);
        if (length < minEdgeLength)
            indexArray.append(mItEdge.index());
    }
    return MS::kSuccess;
}

MStatus MeshChecker::findUnfrozenVertices() {
    // reference
    // https://nccastaff.bournemouth.ac.uk/jmacey/RobTheBloke/www/research/maya/mfn_attributes.htm
    mDagPath.extendToShape();
    MFnDagNode mFnDag(mDagPath);
    MPlug pntsArray = mFnDag.findPlug("pnts");

    MFnMesh fnMesh(mDagPath);
    unsigned int numVerts = fnMesh.numVertices();

    for (unsigned int i = 0; i < numVerts; i++) {
        // MPlug compound = pntsArray.elementByPhysicalIndex(i);
        MPlug compound = pntsArray.elementByLogicalIndex(i);
        float x, y, z;
        if (compound.isCompound()) {
            MPlug plug_x = compound.child(0);
            MPlug plug_y = compound.child(1);
            MPlug plug_z = compound.child(2);

            plug_x.getValue(x);
            plug_y.getValue(y);
            plug_z.getValue(z);

            if (!(x == 0.0 && y == 0.0 && z == 0.0))
                indexArray.append(i);
        }
    }
    return MS::kSuccess;
}

bool MeshChecker::hasVertexPntsAttr() {
	MStatus status;

	mDagPath.extendToShape();
    MFnDagNode dagNode(mDagPath);
    MPlug pntsArray = dagNode.findPlug("pnts");
	MDataHandle dataHandle = pntsArray.asMDataHandle();
	MArrayDataHandle arrayDataHandle(dataHandle);
	MDataHandle outputHandle;

	if (!fix) {
		// Check only.
		while (true) {
			outputHandle = arrayDataHandle.outputValue();
			float3& xyz = outputHandle.asFloat3();
			if (xyz[0] != 0.0)
				return true;
			if (xyz[1] != 0.0)
				return true;
			if (xyz[2] != 0.0)
				return true;
			status = arrayDataHandle.next();
			if (status != MS::kSuccess) {
				break;
			}
		}
	}
	else {
		// Do fix. Reset all vertices pnts attr to 0
		while (true) {
			outputHandle = arrayDataHandle.outputValue();
			outputHandle.set3Double(0.0, 0.0, 0.0);
			status = arrayDataHandle.next();
			if (status != MS::kSuccess) {
				break;
			}
		}
		pntsArray.setMDataHandle(dataHandle);
	}

	return false;
}

MStringArray MeshChecker::setResultString(std::string componentType) {
    MString fullpath = mDagPath.fullPathName();
    MStringArray resultStringArray;
    int index;
    for (unsigned int i = 0; i < indexArray.length(); i++) {
        index = indexArray[i];
        if (componentType == "face") {
            MString name = fullpath + ".f[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "vertex") {
            MString name = fullpath + ".vtx[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "edge") {
            MString name = fullpath + ".e[" + index + "]";
            resultStringArray.append(name);
        } else if (componentType == "uv") {
            MString name = fullpath + ".map[" + index + "]";
            resultStringArray.append(name);
        }
    }
    return resultStringArray;
}

MSyntax MeshChecker::newSyntax() {
    MSyntax syntax;
    syntax.addFlag("-c", "-check", MSyntax::kUnsigned);
    syntax.addFlag("-mfa", "-maxFaceArea", MSyntax::kDouble);
    syntax.addFlag("-mel", "-minEdgeLength", MSyntax::kDouble);
    syntax.addFlag("-fix", "-doFix", MSyntax::kBoolean);
    return syntax;
}

MStatus MeshChecker::doIt(const MArgList &args) {

    MStatus status;
    MArgDatabase argData(syntax(), args);

    if (args.length() == 0) {
        MGlobal::getActiveSelectionList(mList);
    } else if (args.length() > 0) {
        MString argument = args.asString(0, &status);
        if (status != MS::kSuccess) {
            return MStatus::kFailure;
        }
        CHECK_MSTATUS_AND_RETURN_IT(status);
        mList.add(argument);
    } else {
        return MStatus::kFailure;
    }

    mList.getDagPath(0, mDagPath);

    // arg

    if (argData.isFlagSet("-check")) {
        argData.getFlagArgument("-check", 0, checkNumber);
    } else {
        MGlobal::displayError("Check type required.");
        return MS::kFailure;
    }

    if (argData.isFlagSet("-maxFaceArea"))
        argData.getFlagArgument("-maxFaceArea", 0, maxFaceArea);
    else
        maxFaceArea = 0.00001;

    if (argData.isFlagSet("-minEdgeLength"))
        argData.getFlagArgument("-minEdgeLength", 0, minEdgeLength);
    else
        minEdgeLength = 0.000001;

    if (argData.isFlagSet("-fix"))
        argData.getFlagArgument("-fix", 0, fix);
    else
        fix = false;

    switch (checkNumber) {
        case MeshChecker::TRIANGLES:
            status = findTriangles();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("face");
            break;
        case MeshChecker::NGONS:
            status = findNgons();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("face");
            break;
        case MeshChecker::NON_MANIFOLD_EDGES:
            status = findNonManifoldEdges();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("edge");
            break;
        case MeshChecker::LAMINA_FACES:
            status = findLaminaFaces();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("face");
            break;
        case MeshChecker::BI_VALENT_FACES:
            status = findBiValentFaces();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("vertex");
            break;
        case MeshChecker::ZERO_AREA_FACES:
            status = findZeroAreaFaces(maxFaceArea);
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("face");
            break;
        case MeshChecker::MESH_BORDER:
            status = findMeshBorderEdges();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("edge");
            break;
        case MeshChecker::CREASE_EDGE:
            status = findCreaseEDges();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("edge");
            break;
        case MeshChecker::ZERO_LENGTH_EDGES:
            status = findZeroLengthEdges();
            CHECK_MSTATUS_AND_RETURN_IT(status);
            resultArray = setResultString("edge");
            break;
        case MeshChecker::UNFROZEN_VERTICES:
			if (hasVertexPntsAttr())
				MPxCommand::setResult(true);
			else
				MPxCommand::setResult(false);
			return MS::kSuccess;
            break;
        case MeshChecker::TEST:
            break;
        default:
            MGlobal::displayError("Invalid check number");
            return MS::kFailure;
            break;
    }

    MPxCommand::setResult(resultArray);

    return redoIt();
}

MStatus MeshChecker::redoIt() {
    return MS::kSuccess;
}

MStatus MeshChecker::undoIt() {
    return MS::kSuccess;
}

bool MeshChecker::isUndoable() const {
    return false;
}

void *MeshChecker::creator() {
    return new MeshChecker;
}
