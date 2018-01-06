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
    Image {
        id: image
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        height: Fit.fit(57)
        source: "qrc:///resources/images/back-line.png"
    }
    AnimatedImage {
        id: animatedImage
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        width: Fit.fit(200)
        source: "qrc:///resources/images/construction.gif"
    }
    Column {
        anchors.verticalCenterOffset: Fit.fit(-95)
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Fit.fit(10)
        clip: true
        Item { height: Fit.fit(55); width: 1; }
        Text {
            id: informativeText
            text: root.informativeText + waitEffectString
            color: "#e0e4e7"
            anchors.horizontalCenter: parent.horizontalCenter
        }
        ProgressBar {
            id: progressBar
            width: Fit.fit(250)
            height: Fit.fit(5)
            value: progressbarValue
            style: ProgressBarStyle {
                background: Rectangle {
                    radius: Fit.fit(2)
                    border.color: Qt.darker("#135D52", 1.5)
                    color: "#30000000"
                }
                progress: Rectangle {
                    border.color: Qt.darker("#135D52", 1.5)
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
                        visible: control.value < 0.98
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#F4BA48" }
                            GradientStop { position: 1.0; color: Qt.darker("#F4BA48", 1.3) }
                        }
                    }
                }
            }
        }
        RowLayout {
            width: progressBar.width
            spacing: 0
            Text {
                id: mbText
                text: root.mbText
                color: "#e0e4e7"
                horizontalAlignment: Text.AlignLeft
            }
            Item { height: 1; Layout.fillWidth: true }
            Text {
                id: speedText
                text: root.speedText
                color: "#e0e4e7"
                horizontalAlignment: Text.AlignRight
            }
        }
        ProgressBar {
            id: progressBar2
            width: Fit.fit(250)
            height: Fit.fit(5)
            value: progressbarValue2
            style: ProgressBarStyle {
                background: Rectangle {
                    radius: Fit.fit(2)
                    border.color: Qt.darker("#135D52", 1.5)
                    color: "#30000000"
                }
                progress: Rectangle {
                    border.color: Qt.darker("#135D52", 1.5)
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
                        visible: control.value < 0.98
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#F4BA48" }
                            GradientStop { position: 1.0; color: Qt.darker("#F4BA48", 1.3) }
                        }
                    }
                }
            }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Total progress: %" + (progressBar2.value * 100).toFixed(1)
            color: "#e0e4e7"
        }
        Item {
            id: btnCancelOutCont
            width: Fit.fit(150)
            height: Fit.fit(28)
            anchors.horizontalCenter: parent.horizontalCenter
            enabled: btnCancelEnabled
            Behavior on opacity {
                PropertyAnimation {}
            }
            Item {
                id: btnCancelInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnCancel
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.darker("#C2504B", 1.1) }
                        GradientStop { position: 1.0; color: Qt.darker("#C2504B", 1.3) }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Text {
                            text: "Cancel"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnCancelImg.height
                        }
                        Image {
                            id: btnCancelImg
                            source: "qrc:///resources/images/cancel.png"
                            height: btnCancel.height - Fit.fit(12)
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
                anchors.fill: btnCancelInCont
                source: btnCancelInCont
                maskSource: btnCancelMask
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
        Item {
            id: btnOkOutCont
            width: Fit.fit(150)
            height: Fit.fit(28)
            opacity: 0
            visible: false
            anchors.horizontalCenter: parent.horizontalCenter
            Behavior on opacity {
                PropertyAnimation {}
            }
            Item {
                id: btnOkInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnOk
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#8bbb56" }
                        GradientStop { position: 1.0; color: "#6d9f45" }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Text {
                            text: "Ok"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnOkImg.height
                        }
                        Image {
                            id: btnOkImg
                            source: "qrc:///resources/images/cancel.png"
                            height: btnOk.height - Fit.fit(12)
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                    function glow() {
                        btnOkOverlay.color = "#30000000"
                    }
                    function unglow() {
                        btnOkOverlay.color = "#00ffffff"
                    }
                }
                ColorOverlay {
                    id: btnOkOverlay
                    anchors.fill: btnOk
                    source: btnOk
                    color: "#00ffffff"
                }
            }
            Rectangle {
                id: btnOkMask;
                anchors.fill: parent;
                radius: Fit.fit(6);
                visible: false;
            }
            OpacityMask {
                id: btnOkOpMask
                anchors.fill: btnOkInCont
                source: btnOkInCont
                maskSource: btnOkMask
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: btnOk.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onPressed: btnOk.glow()
                onReleased: btnOk.unglow()
                onClicked: btnOkClicked()
            }
        }
        Item { height: Fit.fit(2); width: 1; }
    }
    Image {
        id: downloadComplete
        y: - height - Fit.fit(20)
        width: Fit.fit(200)
        fillMode: Image.PreserveAspectFit
        source: "qrc:///resources/images/bouncing_arrow.png"
        anchors.right:  parent.right
        Behavior on y {
            PropertyAnimation { duration: 800; easing.type: Easing.OutElastic }
        }
    }
    Timer {
        id: waitEffectTimer
        interval: 800
        repeat: true
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
    function startWaitEffect() {
        waitEffectTimer.start()
    }
    function stopWaitEffect() {
        waitEffectTimer.stop()
        waitEffectString = ""
    }
    function showBtnOk() {
        DelayCaller.delayCall(250, function() {
            btnOkOutCont.visible = true
            btnCancelOutCont.visible = false
            btnOkOutCont.opacity = 1
        })
        btnCancelOutCont.opacity = 0

        downloadComplete.y = Qt.binding(function() { return (root.height - downloadComplete.height - Fit.fit(20))})
    }
    function showBtnCancel() {
        DelayCaller.delayCall(250, function() {
            btnCancelOutCont.visible = true
            btnOkOutCont.visible = false
            btnCancelOutCont.opacity = 1
        })
        btnOkOutCont.opacity = 0


        downloadComplete.visible = false
        DelayCaller.delayCall(300, function() {
            downloadComplete.visible = true
        })
        downloadComplete.y = Qt.binding(function() { return (-downloadComplete.height - Fit.fit(20))})
    }
    signal btnCancelClicked()
    signal btnOkClicked()
    property string waitEffectString: ""
    property real progressbarValue: 0
    property real progressbarValue2: 0
    property string informativeText: "Wait"
    property string mbText: "-"
    property string speedText: "-"
    property bool btnCancelEnabled: true
}
