"""
Checker classes
"""

from abc import ABCMeta, abstractmethod
from maya import cmds

from . import error

reload(error)


class BaseChecker:
    """ Base abstract class for each checker """

    __metaclass__ = ABCMeta

    __checkName__ = ""
    __checkLabel__ = ""

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
    def checkLabel(self):
        """ Label property """

        return self.__checkLabel__


class TriangleChecker(BaseChecker):
    """ Triangle checker class """

    __checkName__ = "triangles"
    __checkLabel__ = "Triangles"

    def checkIt(self, objs):
        # type: (str, int) -> (list)

        errors = []

        for obj in objs:
            errs= cmds.checkMesh(obj, c=0)
            errorObj = error.MeshError(obj, errs)
            errors.append(errorObj)

        return errors

    def fixIt(self):
        pass


class NgonChecker(BaseChecker):

    __checkName__ = "ngons"
    __checkLabel__ = "N-gons"

    def checkIt(self, objs):
        # type: (str, int) -> (list)

        errors = []

        for obj in objs:
            errs = cmds.checkMesh(obj, c=1)
            errorObj = error.MeshError(obj, errs)
            errors.append(errorObj)

        return errors

    def fixIt(self):
        pass


class NonmanifoldChecker(BaseChecker):

    __checkName__ = "onmanifoldEdges"
    __checkLabel__ = "Nonmanifold Edges"

    def checkIt(self, objs):
        # type: (str, int) -> (list)

        errors = []

        for obj in objs:
            errs = cmds.checkMesh(obj, c=2)
            errorObj = error.MeshError(obj, errs)
            errors.append(errorObj)

        return errors

    def fixIt(self):
        pass


class LaminaFaceChecker(BaseChecker):

    __checkName__ = "laminafaces"
    __checkLabel__ = "Lamina Faces"

    def checkIt(self, objs):
        # type: (str, int) -> (list)

        errors = []

        for obj in objs:
            errs = cmds.checkMesh(obj, c=3)
            errorObj = error.MeshError(obj, errs)
            errors.append(errorObj)

        return errors

    def fixIt(self):
        pass


class BiValentFaceChecker(BaseChecker):

    __checkName__ = ""
    __checkLabel__ = "Bi-valent Faces"

    def checkIt(self, objs):
        # type: (str, int) -> (list)

        errors = []

        for obj in objs:
            errs = cmds.checkMesh(obj, c=4)
            errorObj = error.MeshError(obj, errs)
            errors.append(errorObj)

        return errors

    def fixIt(self):
        pass


CHECKERS = [
    TriangleChecker,
    NgonChecker,
    NonmanifoldChecker,
    LaminaFaceChecker,
    BiValentFaceChecker]
