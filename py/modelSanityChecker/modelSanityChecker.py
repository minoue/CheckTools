"""

module docstring here
"""

from maya.app.general.mayaMixin import MayaQWidgetDockableMixin
from PySide2 import QtCore, QtWidgets
from maya import OpenMaya
from maya import cmds

from . import checker
reload(checker)


def init():
    """
    Initialize plugins

    """

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


class CheckerWidget(QtWidgets.QWidget):

    RED = """QPushButton{
                background: red;
          }
          """

    GREEN = """QPushButton{
                background: green;
          }
          """

    def __init__(self, chk):
        # type: (checker.BaseChecker) -> (None)
        super(CheckerWidget, self).__init__()

        self.checker = chk
        self.createUI()

        self.setMinimumHeight(100)

    def createUI(self):
        layout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.LeftToRight)

        self.checkButton = QtWidgets.QPushButton(self.checker.checkLabel)
        self.checkButton.setSizePolicy(
            QtWidgets.QSizePolicy.Maximum,
            QtWidgets.QSizePolicy.Expanding)
        self.checkButton.setMinimumWidth(150)
        self.checkButton.clicked.connect(self.check)

        self.errorList = QtWidgets.QListWidget()
        self.errorList.itemClicked.connect(self.errorSelected)

        layout.addWidget(self.checkButton)
        layout.addWidget(self.errorList)

        self.setLayout(layout)

    def check(self):
        sel = cmds.ls(sl=True, fl=True, long=True)

        if len(sel) == 0:
            cmds.warning("Nothing is selected")
            return

        # Clear list items
        self.errorList.clear()

        errsWidgets = self.checker.checkIt(sel)
        for e in errsWidgets:
            if not e.isClean:
                self.errorList.addItem(e)
                self.setColor(self.RED)
            else:
                self.setColor(self.GREEN)

    def errorSelected(self, *args):
        """
        Select error components

        """

        err = args[0]
        cmds.select(err.components, r=True)

    def setColor(self, col):
        """
        Change button color

        """

        self.checkButton.setStyleSheet(col)


class ModelSanityChecker(QtWidgets.QWidget):
    """ Main sanity checker class """

    def __init__(self, parent=None):
        super(ModelSanityChecker, self).__init__(parent)

        self.checkers = [CheckerWidget(i()) for i in checker.CHECKERS]
        self.createUI()

    def createUI(self):
        """
        GUI method

        """

        mainLayout = QtWidgets.QVBoxLayout()

        scroll = QtWidgets.QScrollArea()
        scroll.setWidgetResizable(1)

        scrollLayout = QtWidgets.QVBoxLayout()
        for widget in self.checkers:
            scrollLayout.addWidget(widget)

        content = QtWidgets.QWidget()
        content.setLayout(scrollLayout)

        scroll.setWidget(content)

        checkAllButton = QtWidgets.QPushButton("Check All")
        checkAllButton.clicked.connect(self.checkAll)
        checkAllButton.setMinimumHeight(40)

        mainLayout.addWidget(scroll)
        mainLayout.addWidget(checkAllButton)

        self.setLayout(mainLayout)

    def checkAll(self):
        """
        Check all

        """

        for widget in self.checkers:
            widget.check()


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
        self.tabWidget.addTab(ModelSanityChecker(self), "SanityChecker")

    def layoutUI(self):
        """ Layout widgets """

        mainLayout = QtWidgets.QBoxLayout(QtWidgets.QBoxLayout.TopToBottom)
        mainLayout.addWidget(self.tabWidget)

        self.setLayout(mainLayout)


class MainWindow(MayaQWidgetDockableMixin, QtWidgets.QMainWindow):
    """
    Main window

    """

    def __init__(self, parent=None):
        """ init """

        super(MainWindow, self).__init__(parent)

        self.thisObjectName = "sanityCheckerWindow"
        self.windowTitle = "Sanity Checker"
        self.workspaceControlName = self.thisObjectName + "WorkspaceControl"

        self.setObjectName(self.thisObjectName)
        self.setWindowTitle(self.windowTitle)

        self.setWindowFlags(QtCore.Qt.Window)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)

        # Create and set central widget
        self.cWidget = CentralWidget()
        self.setCentralWidget(self.cWidget)

        self.setupMenu()

    def setupMenu(self):
        """ Setup menu """

        menu = self.menuBar()

        # About
        aboutAction = QtWidgets.QAction("&About", self)
        aboutAction.setStatusTip('About this script')
        aboutAction.triggered.connect(self.showAbout)

        menu.addAction("File")
        helpMenu = menu.addMenu("&Help")
        helpMenu.addAction(aboutAction)

    def showAbout(self):
        """
        About message
        """

        QtWidgets.QMessageBox.about(
            self,
            'About ',
            'test\n')

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
