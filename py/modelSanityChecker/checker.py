"""
Checker classes
"""

from abc import ABCMeta, abstractmethod
from maya import cmds
from maya.api import OpenMaya
from PySide2 import QtWidgets


class Error(QtWidgets.QListWidgetItem):
    """ Custom error object """

    def __init__(self, fullPath, errors=None, parent=None):
        # type: (str, list) -> (None)
        super(Error, self).__init__(parent)

        self.components = errors
        self.longName = fullPath
        self.shortName = fullPath.split("|")[-1]

        self.setText(self.shortName)


class BaseChecker:
    """ Base abstract class for each checker """

    __metaclass__ = ABCMeta
    __category__ = ""
    __name__ = ""
    isWarning = False
    isEnabled = True
    isFixable = False

    def __init__(self):

        self.errors = []

    def __eq__(self, other):
        return self.name == self.name

    def __ne__(self, other):
        return not (self == other)

    def __lt__(self, other):
        return (self.category < other.category)

    @abstractmethod
    def checkIt(self, objs):
        """ Check method """

        pass

    @abstractmethod
    def fixIt(self):
        """ Fix method """

        pass

    @property
    def name(self):
        """ Label property """

        return self.__name__

    @property
    def category(self):
        return self.__category__


class TriangleChecker(BaseChecker):
    """ Triangle checker class """

    __name__ = "Triangles"
    __category__ = "Topology"
    isWarning = True

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []
        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=0)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class NgonChecker(BaseChecker):

    __name__ = "N-gons"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=1)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class NonmanifoldChecker(BaseChecker):

    __name__ = "Nonmanifold Edges"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=2)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class LaminaFaceChecker(BaseChecker):

    __name__ = "Lamina Faces"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=3)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class BiValentFaceChecker(BaseChecker):

    __name__ = "Bi-valent Faces"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=4)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class ZeroAreaFaceChecker(BaseChecker):

    __name__ = "Zero Area Faces"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=5)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class MeshBorderEdgeChecker(BaseChecker):

    __name__ = "Mesh Border Edges"
    isWarning = True
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=6)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class CreaseEdgeChecker(BaseChecker):

    __name__ = "Crease Edges"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=7)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class ZeroLengthEdgeChecker(BaseChecker):

    __name__ = "Zero-length Edges"
    __category__ = "Topology"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=8)
                if errs:
                    errorObj = Error(obj, errs)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class VertexPntsChecker(BaseChecker):

    __name__ = "Vertex Pnts Attribute"
    __category__ = "Attribute"

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                errs = cmds.checkMesh(obj, c=9)
                if errs:
                    errorObj = Error(obj)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class NameChecker(BaseChecker):

    __name__ = "Name"
    __category__ = "Name"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class ShapeNameChecker(BaseChecker):

    __name__ = "ShapeName"
    __category__ = "Name"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class HistoryChecker(BaseChecker):

    __name__ = "History"
    __category__ = "Node"
    isEnabled = True
    isFixable = True

    def checkIt(self, objs):
        # type: (list) -> (list)

        self.errors = []

        for obj in objs:
            hist = cmds.listHistory(obj)
            if len(hist) > 1:
                errorObj = Error(obj)
                self.errors.append(errorObj)

        return self.errors

    def fixIt(self):

        for e in self.errors:
            cmds.delete(e.longName, ch=True)


class TransformChecker(BaseChecker):

    __name__ = "Transform"
    __category__ = "Attribute"

    def checkIt(self, objs):
        # type: (list) -> (list)

        ignore = []

        errors = []

        identity = OpenMaya.MMatrix.kIdentity
        mSel = OpenMaya.MSelectionList()

        for n, i in enumerate(objs):
            mSel.add(i)
            dagPath = mSel.getDagPath(n)
            groupName = dagPath.fullPathName().split("|")[-1]
            if groupName in ignore:
                continue
            dagNode = OpenMaya.MFnDagNode(dagPath)
            transform = dagNode.transformationMatrix()
            if not transform == identity:
                errorObj = Error(i)
                errors.append(errorObj)

        return errors

    def fixIt(self):
        pass


class LockedTransformChecker(BaseChecker):

    __name__ = "Locked Transform"
    __category__ = "Attribute"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class SmoothPreviewChecker(BaseChecker):

    __name__ = "Smooth Preview"
    __category__ = "Attribute"
    isFixable = True

    def checkIt(self, objs):
        # type: (list) -> (list)

        self.errors = []

        for obj in objs:
            meshes = cmds.listRelatives(
                obj, children=True, fullPath=True, type="mesh") or []
            print meshes
            for i in meshes:
                isSmooth = cmds.getAttr(i + ".displaySmoothMesh")
                if isSmooth:
                    err = Error(i)
                    self.errors.append(err)

        return self.errors

    def fixIt(self):

        for e in self.errors:
            cmds.setAttr(e.longName + ".displaySmoothMesh", 0)


class InputConnectionChecker(BaseChecker):

    __name__ = "Input Connections"
    __category__ = "other"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class KeyframeChecker(BaseChecker):

    __name__ = "Keyframe"
    __category__ = "Attribute"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class GhostVertexChecker(BaseChecker):
    """ Ghost vertex checker class """

    __name__ = "Ghost Vertices"
    __category__ = "Topology"

    def __init__(self):
        super(GhostVertexChecker, self).__init__()

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        mSel = OpenMaya.MSelectionList()
        for obj in objs:
            mSel.add(obj)

        for j in range(mSel.length()):
            dagPath = mSel.getDagPath(j)
            try:
                dagPath.extendToShape()
                itVerts = OpenMaya.MItMeshVertex(dagPath)
                badVerts = []
                while not itVerts.isDone():
                    numEdges = itVerts.numConnectedEdges()
                    if numEdges == 0:
                        fullPath = dagPath.fullPathName(
                        ) + ".vtx[{}]".format(itVerts.index())
                        badVerts.append(fullPath)
                    itVerts.next()
                if badVerts:
                    errorObj = Error(dagPath.fullPathName(), badVerts)
                    errors.append(errorObj)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        """ Ghost vertices ARE fixable """

        pass


class IntermediateObjectChecker(BaseChecker):

    __name__ = "Intermediate Object"
    __category__ = "Node"
    isFixable = True

    def checkIt(self, objs):
        # type: (list) -> (list)

        self.errors = []

        for obj in objs:
            children = cmds.listRelatives(
                obj, ad=True, fullPath=True, type="mesh") or []
            for i in children:
                isIntermediate = cmds.getAttr(i + ".intermediateObject")
                if isIntermediate:
                    err = Error(i)
                    self.errors.append(err)
        return self.errors

    def fixIt(self):
        for e in self.errors:
            shape = e.longName

            if cmds.objExists(shape):
                parents = cmds.listRelatives(shape, parent=True) or []
                for i in parents:
                    # Delete history for parents
                    cmds.delete(i, ch=True)
                try:
                    cmds.delete(shape)
                except ValueError:
                    pass


class UnusedLayerChecker(BaseChecker):

    __name__ = "Unused layers"
    __category__ = "other"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class Map1Checker(BaseChecker):

    __name__ = "UVs to map1"
    __category__ = "UV"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class NegativeUvChecker(BaseChecker):

    __name__ = "UVs in negative space"
    __category__ = "UV"

    def checkIt(self, objs):

        errors = []

        mSel = OpenMaya.MSelectionList()

        for obj in objs:
            mSel.add(obj)

        for i in range(mSel.length()):
            dagPath = mSel.getDagPath(i)
            try:
                dagPath.extendToShape()
                badUVs = []
                fnMesh = OpenMaya.MFnMesh(dagPath)
                uArray, vArray = fnMesh.getUVs()

                for index, uv in enumerate(zip(uArray, vArray)):
                    if uv[0] < 0 or uv[1] < 0:
                        fullPath = dagPath.fullPathName() + \
                            ".map[{}]".format(index)
                        badUVs.append(fullPath)
                err = Error(dagPath.fullPathName(), badUVs)
                errors.append(err)

            except RuntimeError:
                # Not mesh. Do no nothing
                pass

        return errors

    def fixIt(self):
        pass


class UdimIntersectionChecker(BaseChecker):

    __name__ = "UDIM intersection"
    __category__ = "UV"
    isEnabled = False

    def checkIt(self, objs):

        errors = []

        mSel = OpenMaya.MSelectionList()

        for obj in objs:
            mSel.add(obj)

        for i in range(mSel.length()):
            dagPath = mSel.getDagPath(i)
            try:
                dagPath.extendToShape()

            except RuntimeError:
                # Not mesh. Do no nothing
                pass

        return errors

    def fixIt(self):
        pass


class UnassignedUvChecker(BaseChecker):

    __name__ = "Unassigned UVs"
    __category__ = "UV"
    isEnabled = False

    def checkIt(self, objs):

        errors = []

        mSel = OpenMaya.MSelectionList()

        for obj in objs:
            mSel.add(obj)

        for i in range(mSel.length()):
            dagPath = mSel.getDagPath(i)
            try:
                dagPath.extendToShape()

            except RuntimeError:
                # Not mesh. Do no nothing
                pass

        return errors

    def fixIt(self):
        pass


class UnmappedPolygonFaceChecker(BaseChecker):

    __name__ = "Unmapped polygon faces"
    __category__ = "UV"
    isEnabled = False

    def checkIt(self, objs):

        errors = []

        mSel = OpenMaya.MSelectionList()

        for obj in objs:
            mSel.add(obj)

        for i in range(mSel.length()):
            dagPath = mSel.getDagPath(i)
            try:
                dagPath.extendToShape()

            except RuntimeError:
                # Not mesh. Do no nothing
                pass

        return errors

    def fixIt(self):
        pass


class ZeroAreaUVFaceChecker(BaseChecker):

    __name__ = "Zero area UV Faces"
    __category__ = "UV"
    isEnabled = False

    def checkIt(self, objs):

        errors = []

        mSel = OpenMaya.MSelectionList()

        for obj in objs:
            mSel.add(obj)

        for i in range(mSel.length()):
            dagPath = mSel.getDagPath(i)
            try:
                dagPath.extendToShape()

            except RuntimeError:
                # Not mesh. Do no nothing
                pass

        return errors

    def fixIt(self):
        pass


class UvOverlapChecker(BaseChecker):

    __name__ = "UV Overlaps"
    __category__ = "UV"
    isEnabled = False

    def checkIt(self, objs):

        errors = []

        mSel = OpenMaya.MSelectionList()

        for obj in objs:
            mSel.add(obj)

        for i in range(mSel.length()):
            dagPath = mSel.getDagPath(i)
            try:
                dagPath.extendToShape()

            except RuntimeError:
                # Not mesh. Do no nothing
                pass

        return errors

    def fixIt(self):
        pass


class SelectionSetChecker(BaseChecker):

    __name__ = "Selection Sets"
    __category__ = "other"
    isEnabled = False

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                pass
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


class ColorSetChecker(BaseChecker):

    __name__ = "Color Sets"
    __category__ = "other"
    isFixable = True

    def checkIt(self, objs):
        # type: (list) -> (list)

        # Reset result
        self.errors = []

        for obj in objs:
            try:
                allColorSets = cmds.polyColorSet(
                    obj, q=True, allColorSets=True)
                if allColorSets is None:
                    continue
                else:
                    err = Error(obj)
                    self.errors.append(err)
            except RuntimeError:
                pass

        return self.errors

    def fixIt(self):
        for i in self.errors:
            allSets = cmds.polyColorSet(i.longName, q=True, allColorSets=True) or []
            for s in allSets:
                cmds.polyColorSet(i.longName, delete=True, colorSet=s)


CHECKERS = [
    NameChecker,
    ShapeNameChecker,
    HistoryChecker,
    TransformChecker,
    LockedTransformChecker,
    SmoothPreviewChecker,
    InputConnectionChecker,
    KeyframeChecker,
    TriangleChecker,
    NgonChecker,
    NonmanifoldChecker,
    LaminaFaceChecker,
    BiValentFaceChecker,
    ZeroAreaFaceChecker,
    MeshBorderEdgeChecker,
    CreaseEdgeChecker,
    ZeroLengthEdgeChecker,
    VertexPntsChecker,
    GhostVertexChecker,
    IntermediateObjectChecker,
    UnusedLayerChecker,
    Map1Checker,
    NegativeUvChecker,
    UdimIntersectionChecker,
    UnassignedUvChecker,
    UnmappedPolygonFaceChecker,
    ZeroAreaUVFaceChecker,
    UvOverlapChecker,
    SelectionSetChecker,
    ColorSetChecker]
