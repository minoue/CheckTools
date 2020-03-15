from PySide2 import QtWidgets


class MeshError(QtWidgets.QListWidgetItem):

    def __init__(self, fullPath, errors=[], parent=None):
        # type: (str, list) -> (None)
        super(MeshError, self).__init__(parent)

        self.components = errors
        self.longName = fullPath
        self.shortName = fullPath.split("|")[-1]

        if not self.components:
            self.isClean = True
        else:
            self.isClean = False

        self.setText(self.shortName)


class UVError(QtWidgets.QListWidgetItem):

    def __init__(self, parent=None):
        super(UVError, self).__init__(parent)
