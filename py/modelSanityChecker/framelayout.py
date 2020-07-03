""" qt framelayout sample """

from PySide2 import QtWidgets, QtCore
from . import icon
reload(icon)


class TitleLabel(QtWidgets.QLabel):

    clicked = QtCore.Signal()

    def __init__(self, text="", parent=None):
        super(TitleLabel, self).__init__(parent)

        self.setText(text)

    def mousePressEvent(self, event):
        self.clicked.emit()


class FrameLayout(QtWidgets.QWidget):

    def __init__(self, title="", parent=None):
        super(FrameLayout, self).__init__(parent)

        self.baseTitle = title
        self.rightArrow = u"\u25b6 "
        self.downArrow = u"\u25bc "

        titleLayout = QtWidgets.QHBoxLayout()
        self.title = self.rightArrow + title

        self.titleLabel = TitleLabel(self.title)
        self.titleLabel.setSizePolicy(
            QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        self.titleLabel.clicked.connect(self.titleClicked)
        self.statusIconLabel = QtWidgets.QLabel()
        self.statusIconLabel.setPixmap(icon.neutralIconPixmap)

        self.childrenWidget = QtWidgets.QWidget()
        self.childrenLayout = QtWidgets.QVBoxLayout()
        self.childrenLayout.setContentsMargins(0, 0, 0, 0)

        self.childrenWidget.setLayout(self.childrenLayout)

        titleLayout.addWidget(self.statusIconLabel)
        titleLayout.addWidget(self.titleLabel)

        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addLayout(titleLayout)
        layout.addWidget(self.childrenWidget)
        self.setLayout(layout)

        # Close frame by default
        self.childrenWidget.hide()

    def titleClicked(self):
        """ 
        title clicked action 

        """

        newTitle = ""

        if self.childrenWidget.isVisible():
            self.childrenWidget.hide()
            newTitle = self.rightArrow + self.baseTitle
        else:
            self.childrenWidget.show()
            newTitle = self.downArrow + self.baseTitle

        self.titleLabel.setText(newTitle)

    def addWidget(self, widget):
        # type: (QtWidgets.QWidget) -> None
        """
        Add widgets

        """

        self.childrenLayout.addWidget(widget)

    def addLayout(self, layout):
        self.childrenLayout.addLayout(layout)

    def setStatusIcon(self, status):
        # type: (str) -> None
        """
        Change status icon

        """

        if status == "good":
            self.statusIconLabel.setPixmap(icon.goodIconPixmap)
        elif status == "bad":
            self.statusIconLabel.setPixmap(icon.errorIconPixmap)
        elif status == "warning":
            self.statusIconLabel.setPixmap(icon.warningIconPixmap)
        elif status == "neutral":
            self.statusIconLabel.setPixmap(icon.neutralIconPixmap)
        else:
            pass
