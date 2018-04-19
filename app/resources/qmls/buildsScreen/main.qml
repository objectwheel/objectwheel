import QtQuick 2.7
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.1

Rectangle {
    id: root
    color: "#135D52"
    Image {
        id: helmetIcon
        source: "qrc:///images/helmet.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 15
        height: 50
        fillMode: Image.PreserveAspectFit
    }
    Text {
        id: title
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: helmetIcon.bottom
        anchors.topMargin: 5
        font.pixelSize: 28
        text: "Objectwheel Builds"
        color: "#ececec"
        font.weight: Font.ExtraLight
    }
    Text {
        id: selectmsg
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: title.bottom
        anchors.topMargin: 15
        font.pixelSize: 18
        text: "Build inprogress"
        color: "#ececec"
        font.weight: Font.Light
    }
    ProgressPage {
        id: progressPage
        anchors { right: parent.right; left: parent.left; bottom: parent.bottom; top: selectmsg.bottom; }
        anchors.topMargin: 8
        clip: true
    }
    property alias progressPage: progressPage
}
