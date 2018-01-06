import QtQuick 2.7
import QtGraphicalEffects 1.0
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller

Item {
    id: root
    visible: false
    opacity: 0
    Behavior on opacity {
        PropertyAnimation { id: anim}
    }
    Rectangle {
        id: base
        color: "#D0D4D7"
        clip: true
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Fit.fit(25)
        height: Fit.fit(30)
        width: 200
        radius: Fit.fit(10)
        Image {
            id: img
            anchors.topMargin: Fit.fit(2)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            source: "qrc:///resources/images/sign_warning.png"
            fillMode: Image.PreserveAspectFit
            height: Fit.fit(16)
            mipmap: true
        }
        Text {
            id: text
            anchors.top: img.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.rightMargin: Fit.fit(12)
            anchors.leftMargin: Fit.fit(12)
            clip: true
            color: Qt.lighter("#2E3A41", 1.55)
            text: "       "
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
        visible: false
    }
    DropShadow {
        anchors.fill: base
        horizontalOffset: 0
        verticalOffset: Fit.fit(1)
        radius: (5)
        samples: (14)
        color: "#30000000"
        source: base
    }
    QtObject {
        id: d
        property Timer timer: Timer {
            interval: root.duration
            onTriggered: {
                root.opacity = 0
                DelayCaller.delayCall(anim.duration, function() {
                    if (root.opacity == 0) {
                        root.visible = false
                    }
                })
            }
        }
    }
    function show() {
        root.visible = true
        root.opacity = 1
        d.timer.restart()
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            d.timer.stop()
            root.opacity = 0
            DelayCaller.delayCall(anim.duration, function() {
                if (root.opacity == 0) {
                    root.visible = false
                }
            })
        }
    }
    property int duration: 5000
    property alias text: text
    property alias base: base
}
