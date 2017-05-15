import QtQuick 2.0
import QtGraphicalEffects 1.0
import "../fit.js" as Fit

Item {

    Rectangle {
        id: mask
        anchors.fill: parent
        radius: Fit.fit(8)
        visible: false
    }

    Item {
        id: buttonsContainer
        anchors.fill: parent
        visible: false

        Rectangle {
            id: btnLeft
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom}
            width: btnRight.width
            visible: false
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#5bc5f8" }
                GradientStop { position: 1.0; color: "#2491F9" }
            }

            Row {
                anchors.centerIn: parent
                spacing: Fit.fit(6)
                Text {
                    id: txtLoad
                    text: "New Project"
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    height: imgLoad.height
                    font.pixelSize: Fit.fit(13)
                }

                Image {
                    id: imgLoad
                    source: "qrc:///resources/images/new.png"
                    height: btnLeft.height - Fit.fit(14)
                    fillMode: Image.PreserveAspectFit
                }
            }

            function glow() {
                leftOverlay.color = "#30000000"
            }
            function unglow() {
                leftOverlay.color = "#00ffffff"
            }
        }

        Rectangle {
            id: btnRight
            enabled: !loadButtonDisabled
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom}
            width: Math.floor(parent.width / 2.0)
            visible: false
            gradient: Gradient {
                GradientStop { position: 0.0; color: loadButtonDisabled ? "#888888" : "#8bbb56" }
                GradientStop { position: 1.0; color: loadButtonDisabled ? "#656565" : "#6d9f45" }
            }

            Row {
                anchors.centerIn: parent
                spacing: Fit.fit(6)
                Image {
                    id: imgLoad2
                    source: "qrc:///resources/images/load.png"
                    height: btnRight.height - Fit.fit(14)
                    fillMode: Image.PreserveAspectFit
                }
                Text {
                    id: txtLoad2
                    text: "Load Project"
                    color: "white"
                    font.pixelSize: Fit.fit(13)
                    verticalAlignment: Text.AlignVCenter
                    height: imgLoad.height
                }
            }

            function glow() {
                rightOverlay.color = "#30000000"
            }
            function unglow() {
                rightOverlay.color = "#00ffffff"
            }
        }

        ColorOverlay {
            id: rightOverlay
            anchors.fill: btnRight
            source: btnRight
            color: "#00ffffff"
        }

        ColorOverlay {
            id: leftOverlay
            anchors.fill: btnLeft
            source: btnLeft
            color: "#00ffffff"
        }
    }

    OpacityMask {
        id: opMask
        visible: false
        anchors.fill: buttonsContainer
        source: buttonsContainer
        maskSource: mask
    }

    DropShadow {
        anchors.fill: opMask
        horizontalOffset: 0
        verticalOffset: Fit.fit(1)
        radius: (6.0)
        samples: (14)
        color: "#80000000"
        source: opMask
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: {
            if (loadButtonDisabled && mouseX >= parent.width / 2.0) return Qt.ArrowCursor
            else return Qt.PointingHandCursor
        }
        onPressed: {
            if (mouse.x < parent.width / 2.0) btnLeft.glow()
            else if (!loadButtonDisabled) btnRight.glow()
        }
        onReleased: {
            btnLeft.unglow(); btnRight.unglow()
        }
        onClicked: {
            if (mouse.x < parent.width / 2.0) newButtonClicked()
            else if (!loadButtonDisabled) loadButtonClicked()
        }
    }
    Rectangle {
        width: Fit.fit(1)
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; bottom: parent.bottom}
        color: "#30545454"
    }
    property bool loadButtonDisabled : false
    signal newButtonClicked()
    signal loadButtonClicked()
}
