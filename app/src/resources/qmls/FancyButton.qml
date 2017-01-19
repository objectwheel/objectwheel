import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import "fit.js" as Fit

Rectangle {
    id: root
    color: "transparent"

    Rectangle {
        id: checkedEffect
        anchors.fill: parent
        radius: Math.min(width / 2.0, height / 2.0)
        color: "#30000000"
        visible: checkable ? checked : false
    }

    Image {
        id: iconImage
        anchors.fill: parent
        anchors.margins: Fit.fit(4)
        sourceSize: Qt.size(width, height)
        visible: !pressingEffect.visible
    }

    ColorOverlay {
        id: pressingEffect
        anchors.fill: iconImage
        source: iconImage
        color: "#65000000"
        visible: {
            if (!root.enabled) return true;
            else if (checkable) return false
            else return mouse.pressed
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        cursorShape: cursorMark
        onClicked: {
            if (checkable) checked = !checked
            root.clicked();
        }
    }

    property bool checked: false
    property bool checkable: false
    property int cursorMark: Qt.PointingHandCursor
    property alias iconSource: iconImage.source
    signal clicked()
}
