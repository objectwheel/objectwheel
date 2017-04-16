import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import "fit.js" as Fit

Item {
    id: root
    Rectangle {
        id: base
        anchors.fill: parent
        visible: iconButton ? false : !pressingEffect.visible
    }
    Rectangle {
        id: checkedEffect
        anchors.fill: parent
        radius: iconButton ? Math.min(width / 2.0, height / 2.0) : base.radius
        color: "#30000000"
        visible: checkable ? checked : false
    }

    Image {
        id: iconImage
        anchors.fill: parent
        anchors.margins: Fit.fit(4)
        sourceSize: Qt.size(width, height)
        visible: !iconButton ? false : !pressingEffect.visible
    }

    ColorOverlay {
        id: pressingEffect
        anchors.fill: !iconButton ? base : iconImage
        source: !iconButton ? base : iconImage
        color: pressedColor
        visible: {
            if (!root.enabled) return true;
            else if (checkable) return false
            else return mouse.pressed
        }
    }

    Text {
        id: text
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    MouseArea {
        id: mouse
        hoverEnabled: true
        anchors.fill: parent
        cursorShape: cursorMark
        onClicked: {
            if (checkable) checked = !checked
            root.clicked();
        }
        onPressed: { root.pressed(); pressing = true; }
        onReleased: { root.released(); pressing = false; }
        onPressAndHold: root.pressAndHold()
    }

    property bool pressing: false
    property bool iconButton: true
    property color pressedColor: "#15000000"
    property alias text: text
    property alias base: base
    property bool checked: false
    property bool checkable: false
    property int cursorMark: Qt.PointingHandCursor
    property alias iconSource: iconImage.source
    signal clicked()
    signal pressed()
    signal released()
    signal pressAndHold()
}
