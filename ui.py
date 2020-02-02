from maya.app.general.mayaMixin import MayaQWidgetDockableMixin
from PySide2 import QtCore, QtWidgets
from maya import OpenMaya
# from maya import OpenMayaUI
from maya import cmds
# import shiboken2
from functools import partial


CHECKS = [
    "Triangles",
    "N-gons",
    "Non-manifold edges",
    "Lamina faces"]


def init():
    if not cmds.pluginInfo("meshChecker", q=True, loaded=True):
        try:
            cmds.loadPlugin("meshChecker")
        except RuntimeError:
            raise RuntimeError("Failed to load plugin")

    if not cmds.pluginInfo("uvChecker", q=True, loaded=True):
        try:
            cmds.loadPlugin("uvChecker")
        except RuntimeError:
            raise RuntimeError("Failed to load plugin")

    if not cmds.pluginInfo("findUvOverlaps", q=True, loaded=True):
        try:
            cmds.loadPlugin("findUvOverlaps")
        except RuntimeError:
            raise RuntimeError("Failed to load plugin")


class MeshChecker(QtWidgets.QWidget):
    """ Contents widget for tabs """

    def __init__(self, parent=None):
        """ Init """

        super(MeshChecker, self).__init__(parent)

        triBtn = QtWidgets.QPushButton("Triangles")
        triBtn.clicked.connect(partial(self.run, 0))

        ngonBtn = QtWidgets.QPushButton("N-gons")
        ngonBtn.clicked.connect(partial(self.run, 1))

        manifoldBtn = QtWidgets.QPushButton("Non-manifold edges")
        manifoldBtn.clicked.connect(partial(self.run, 2))

        laminaBtn = QtWidgets.QPushButton("Lamina faces")
        laminaBtn.clicked.connect(partial(self.run, 3))

        layout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        layout.setAlignment(QtCore.Qt.AlignTop)
        layout.addWidget(triBtn)
        layout.addWidget(ngonBtn)
        layout.addWidget(manifoldBtn)
        layout.addWidget(laminaBtn)

        self.setLayout(layout)

    def run(self, checkNum):
        """ run check command """

        sel = cmds.ls(sl=True, fl=True, long=True)
        if not len(sel) == 0:
            result = cmds.checkMesh(sel[0], c=checkNum)

            if len(result) == 0:
                OpenMaya.MGlobal.displayInfo(
                    "Good. No {} are found".format(CHECKS[checkNum]))
            else:
                cmds.select(result, r=True)
                numErrors = len(result)
                OpenMaya.MGlobal.displayWarning(
                    "{} {} are found".format(numErrors, CHECKS[checkNum]))


class UVChecker(QtWidgets.QWidget):
    """ Contents widget for tabs """

    def __init__(self, parent=None):
        """ Init """

        super(UVChecker, self).__init__(parent)


class CentralWidget(QtWidgets.QWidget):
    """ Central widget """

    def __init__(self, parent=None):
        """ Init """

        super(CentralWidget, self).__init__(parent)

        self.createUI()
        self.layoutUI()

    def createUI(self):
        """ Crete widgets """

        self.tabWidget = QtWidgets.QTabWidget()
        self.tabWidget.addTab(MeshChecker(self), "Mesh Checker")
        self.tabWidget.addTab(UVChecker(self), "UV Checker")

    def layoutUI(self):
        """ Layout widgets """

        mainLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        mainLayout.addWidget(self.tabWidget)

        self.setLayout(mainLayout)


class MainWindow(MayaQWidgetDockableMixin, QtWidgets.QMainWindow):

    def __init__(self, parent=None):
        """ init """

        super(MainWindow, self).__init__(parent)

        self.thisObjectName = "sanityCheckerWindow"
        self.WindowTitle = "Sanity Checker"
        self.workspaceControlName = self.thisObjectName + "WorkspaceControl"

        self.setObjectName(self.thisObjectName)
        self.setWindowTitle(self.WindowTitle)

        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        # Create and set central widget
        self.cw = CentralWidget()
        self.setCentralWidget(self.cw)

        self.setupMenu()

    def setupMenu(self):
        """ Setup menu """

        menu = self.menuBar()

        # About
        aboutAction = QtWidgets.QAction("&About", self)
        aboutAction.setStatusTip('About this script')
        aboutAction.triggered.connect(self.showAbout)

        menu.addAction("File")
        help_menu = menu.addMenu("&Help")
        help_menu.addAction(aboutAction)

    def showAbout(self):
        """ about message """

        QtWidgets.QMessageBox.about(
            self,
            'About ',
            'Awesome window\n')

    def run(self):
        try:
            cmds.deleteUI(self.workspaceControlName)
        except RuntimeError:
            pass

        self.show(dockable=True)
        cmds.workspaceControl(
            self.workspaceControlName,
            edit=True,
            dockToControl=['Outliner', 'right'])
        self.raise_()


def main():
    try:
        init()
    except RuntimeError:
        return

    w = MainWindow()
    w.run()


if __name__ == "__main__":
    main()
