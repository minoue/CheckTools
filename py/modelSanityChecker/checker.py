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
    __name__ = ""
    isWarning = False
    isEnabled = True

    def __init__(self):
        pass

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


class TriangleChecker(BaseChecker):
    """ Triangle checker class """

    __name__ = "Triangles"
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


class TransformChecker(BaseChecker):

    __name__ = "Transform"

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


class InputConnectionChecker(BaseChecker):

    __name__ = "Input Connections"
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

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            children = cmds.listRelatives(obj, ad=True, fullPath=True, type="mesh") or []
            for i in children:
                isIntermediate = cmds.getAttr(i + ".intermediateObject")
                if isIntermediate:
                    err = Error(i)
                    errors.append(err)
        return errors

    def fixIt(self):
        pass


class UnusedLayerChecker(BaseChecker):

    __name__ = "Unused layers"
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


class SelectionSetChecker(BaseChecker):

    __name__ = "Selection Sets"
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

    def checkIt(self, objs):
        # type: (list) -> (list)

        errors = []

        for obj in objs:
            try:
                allColorSets = cmds.polyColorSet(
                    obj, q=True, allColorSets=True)
                if allColorSets is None:
                    continue
                else:
                    err = Error(obj)
                    errors.append(err)
            except RuntimeError:
                pass

        return errors

    def fixIt(self):
        pass


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
    SelectionSetChecker,
    ColorSetChecker]
