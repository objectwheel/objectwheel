import QtQuick 2.7
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.1
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller
import "../loginScreen/"

Rectangle {
    id: root
    color: swipeView.currentIndex == 0 ? "#E0E4E7" : "#135D52"
    Behavior on color {
        ColorAnimation { duration: 300 }
    }

    Image {
        id: helmetIcon
        source: "qrc:///resources/images/helmet.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Fit.fit(15)
        height: Fit.fit(48)
        fillMode: Image.PreserveAspectFit
        visible: false
    }
    DropShadow {
        id: shdHelmet
        anchors.fill: helmetIcon
        horizontalOffset: 0
        verticalOffset: Fit.fit(1)
        radius: 5.0
        samples: 14
        color: "#30000000"
        source: helmetIcon
    }
    Image {
        id: title
        source: "qrc:///resources/images/builds.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: shdHelmet.bottom
        anchors.topMargin: Fit.fit(5)
        height: Fit.fit(28)
        fillMode: Image.PreserveAspectFit
        visible: false
    }
    Image {
        id: selectmsg
        source: swipeView.currentIndex == 0 ? "qrc:///resources/images/selectmsg.png" : "qrc:///resources/images/build_inprogress.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: title.bottom
        anchors.topMargin: Fit.fit(15)
        height: Fit.fit(17)
        fillMode: Image.PreserveAspectFit
    }
    ColorOverlay {
        anchors.fill: title
        source: title
        color: swipeView.currentIndex == 1 ? "#e0e4e7" : "#01000000"
        Behavior on color {
            ColorAnimation { duration: 300 }
        }
    }
    SwipeView {
        id: swipeView
        anchors { right: parent.right; left: parent.left; bottom: parent.bottom; top: selectmsg.bottom; }
        anchors.topMargin: Fit.fit(8)
        interactive: false
        clip: true

        onCurrentIndexChanged: {
            swipeView.currentIndex == 1 ? progressPage.showAnimations() : progressPage.hideAnimations()
        }

        BuildPage {
            id: buildPage
            clip: true
        }

        ProgressPage {
            id: progressPage
            clip: true
        }
    }
    Toast {
        id: toast
        anchors.fill: parent
    }
    Timer {
        running: true
//        onTriggered: swipeView.currentIndex = 1
        interval: 3000
    }

    property alias toast: toast
    property alias buildPage: buildPage
    property alias progressPage: progressPage
    property alias swipeView: swipeView
}
