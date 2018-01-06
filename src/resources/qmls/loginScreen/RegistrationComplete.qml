import QtQuick 2.7
import QtGraphicalEffects 1.0
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller

Item {
    id: root

    AnimatedImage {
        id: img
        anchors.horizontalCenter: parent.horizontalCenter
        y: (parent.height - height) - (parent.height - height) / 1.25
        source: "qrc:///resources/images/complete.gif"
        fillMode: Image.PreserveAspectFit
        width: Fit.fit(100)
        visible: false
    }

    Text {
        id: owText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: img.bottom
        anchors.topMargin: Fit.fit(20)
        text: "Objectwheel"
        color: "#2E3A41"
        font.weight: Font.ExtraLight
        font.pixelSize: Fit.fit(28)
    }

    Text {
        id: thanksText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: owText.bottom
        anchors.topMargin: Fit.fit(30)
        text: "Thank you for registration."
        font.pixelSize: Fit.fit(15)
        font.bold: true
        color: "#65A35C"
    }

    Text {
        id: infoText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: thanksText.bottom
        text: "Please check your mailbox to\nverify your account."
        color: "#304050"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    Item {
        id: btnOkayOutCont
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: infoText.bottom
        anchors.topMargin: Fit.fit(30)
        width: Fit.fit(140)
        height: Fit.fit(28)
        Item {
            id: btnOkayInCont
            anchors.fill: parent
            visible: false
            Rectangle {
                id: btnOkay
                anchors.fill: parent
                visible: false
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#77C06C" }
                    GradientStop { position: 1.0; color: Qt.darker("#77C06C", 1.2) }
                }
                Row {
                    anchors.centerIn: parent
                    spacing: Fit.fit(6)
                    Image {
                        id: btnOkayImg
                        source: "qrc:///resources/images/ok.png"
                        height: btnOkay.height - Fit.fit(12)
                        fillMode: Image.PreserveAspectFit
                    }
                    Text {
                        text: "Ok"
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        height: btnOkayImg.height
                    }
                }
                signal clicked()
                function glow() {
                    btnOkayOverlay.color = "#30000000"
                }
                function unglow() {
                    btnOkayOverlay.color = "#00ffffff"
                }
            }
            ColorOverlay {
                id: btnOkayOverlay
                anchors.fill: btnOkay
                source: btnOkay
                color: "#00ffffff"
            }
        }
        Rectangle {
            id: btnOkayMask;
            anchors.fill: parent;
            radius: Fit.fit(6);
            visible: false;
        }
        OpacityMask {
            id: btnOkayOpMask
            anchors.fill: btnOkayInCont
            source: btnOkayInCont
            maskSource: btnOkayMask
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: btnOkay.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onPressed: btnOkay.glow()
            onReleased: btnOkay.unglow()
            onClicked: btnOkay.clicked()
        }
    }

    function animate() {
        img.visible = false
        DelayCaller.delayCall(400, function() {
            img.visible = true
            img.currentFrame = 0
            img.playing = true
        })
    }
    property alias btnOkay: btnOkay
}
