import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import Qt.labs.platform 1.0
import "../fit.js" as Fit
import "../filemanager.js" as FileManager
import "../delaycaller.js" as DelayCaller

Item {
    id: root

    Column {
        anchors.verticalCenterOffset: Fit.fit(-85)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Fit.fit(10)
        Text {
            id: informativeText
            text: root.informativeText + waitEffectString
            color: "#e0e4e7"
            font.pixelSize: Fit.fit(14)
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Item { height: Fit.fit(10); width: 1; }
        ProgressBar {
            id: progressBar
            width: Fit.fit(250)
            height: Fit.fit(5)
            value: 0.4
            style: ProgressBarStyle {
                background: Rectangle {
                    radius: Fit.fit(2)
                    border.color: Qt.darker("#135D52", 1.3)
                    color: "#25000000"
                }
                progress: Rectangle {
                    border.color: Qt.darker("#135D52", 1.3)
                    radius: Fit.fit(2)
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#F4BA48" }
                        GradientStop { position: 1.0; color: Qt.darker("#F4BA48", 1.3) }
                    }
                    Rectangle {
                        height: parent.height
                        width: Fit.fit(10)
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.topMargin: Fit.fit(1)
                        anchors.bottomMargin: Fit.fit(1)
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#F4BA48" }
                            GradientStop { position: 1.0; color: Qt.darker("#F4BA48", 1.3) }
                        }
                    }
                }
            }
        }
        RowLayout {
            width: parent.width
            Text {
                id: mbText
                text: root.mbText
                color: "#e0e4e7"
                font.pixelSize: Fit.fit(12)
            }
            Item { height: 1; Layout.fillWidth: true }
            Text {
                id: speedText
                text: root.speedText
                color: "#e0e4e7"
                font.pixelSize: Fit.fit(12)
            }
        }
        Item { height: Fit.fit(10); width: 1; }
        Item {
            id: btnCancelOutCont
            width: Fit.fit(150)
            height: Fit.fit(28)
            anchors.horizontalCenter: parent.horizontalCenter
            Item {
                id: btnCancelInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnCancel
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#BB2023" }
                        GradientStop { position: 1.0; color: Qt.darker("#BB2023", 1.2) }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Text {
                            text: "Cancel"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnCancelImg.height
                            font.pixelSize: Fit.fit(13)
                        }
                        Image {
                            id: btnCancelImg
                            source: "qrc:///resources/images/cancel.png"
                            height: btnCancel.height - Fit.fit(14)
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                    function glow() {
                        btnCancelOverlay.color = "#30000000"
                    }
                    function unglow() {
                        btnCancelOverlay.color = "#00ffffff"
                    }
                }
                ColorOverlay {
                    id: btnCancelOverlay
                    anchors.fill: btnCancel
                    source: btnCancel
                    color: "#00ffffff"
                }
            }
            Rectangle {
                id: btnCancelMask;
                anchors.fill: parent;
                radius: Fit.fit(6);
                visible: false;
            }
            OpacityMask {
                id: btnCancelOpMask
                visible: false
                anchors.fill: btnCancelInCont
                source: btnCancelInCont
                maskSource: btnCancelMask
            }
            DropShadow {
                anchors.fill: btnCancelOpMask
                horizontalOffset: 0
                verticalOffset: Fit.fit(1)
                radius: Fit.fit(6.0)
                samples: Fit.fit(14)
                color: "#80000000"
                source: btnCancelOpMask
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: btnCancel.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onPressed: btnCancel.glow()
                onReleased: btnCancel.unglow()
                onClicked: btnCancelClicked()
            }
        }

    }
    Image {
        opacity: animationOpacity
        Behavior on opacity {
            PropertyAnimation { duration: 400 }
        }
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: Fit.fit(57)
        source: "qrc:///resources/images/back-line.png"
    }
    AnimatedImage {
        opacity: animationOpacity
        Behavior on opacity {
            PropertyAnimation { duration: 400 }
        }
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        width: Fit.fit(200)
        source: "qrc:///resources/images/construction.gif"
    }
    Timer {
        interval: 800
        repeat: true
        running: true
        onTriggered: {
            if (waitEffectString.length < 1) {
                waitEffectString = ".";
            } else if (waitEffectString.length < 2) {
                waitEffectString = "..";
            } else if (waitEffectString.length < 3) {
                waitEffectString = "...";
            } else {
                waitEffectString = "";
            }
        }
    }
    function showAnimations() {
        DelayCaller.delayCall(500, function() {
            animationOpacity = 1
        })
    }
    function hideAnimations() {
        animationOpacity = 0
    }
    signal btnCancelClicked()
    property real animationOpacity: 0
    property string waitEffectString: ""
    property string informativeText: "Project Uploading"
    property string mbText: "12.2/59.1 Mb"
    property string speedText: "128.5 Kbps"
}
