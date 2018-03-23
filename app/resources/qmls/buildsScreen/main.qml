import QtQuick 2.7
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.1
import "../fit.js" as Fit

Rectangle {
    id: root
    color: "#135D52"
    Image {
        id: helmetIcon
        source: "qrc:///resources/images/helmet.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Fit.fit(15)
        height: Fit.fit(50)
        fillMode: Image.PreserveAspectFit
    }
    Text {
        id: title
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: helmetIcon.bottom
        anchors.topMargin: Fit.fit(5)
        font.pixelSize: Fit.fit(28)
        text: "Objectwheel Builds"
        color: "#e0e4e7"
        font.weight: Font.ExtraLight
    }
    Text {
        id: selectmsg
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: title.bottom
        anchors.topMargin: Fit.fit(15)
        font.pixelSize: Fit.fit(18)
        text: "Build inprogress"
        color: "#e0e4e7"
        font.weight: Font.Light
    }
    ProgressPage {
        id: progressPage
        anchors { right: parent.right; left: parent.left; bottom: parent.bottom; top: selectmsg.bottom; }
        anchors.topMargin: Fit.fit(8)
        clip: true
    }
    property alias progressPage: progressPage
}
