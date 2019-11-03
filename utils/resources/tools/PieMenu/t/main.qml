import QtQuick 2.12
import QtQuick.Extras 1.4
import QtQuick.Controls 1.4

PieMenu {
    id: pieMenu
    visible: true
    MenuItem {
        text: qsTr("Left")
        iconSource: "qrc:/qt-project.org/styles/commonstyle/images/left-128"
        onTriggered: print(text)
    }
    MenuItem {
        text: qsTr("Up")
        iconSource: "qrc:/qt-project.org/styles/commonstyle/images/up-128"
        onTriggered: print(text)
    }
    MenuItem {
        text: qsTr("Right")
        iconSource: "qrc:/qt-project.org/styles/commonstyle/images/right-128"
        onTriggered: print(text)
    }
}
