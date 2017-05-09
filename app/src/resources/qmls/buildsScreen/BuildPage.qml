import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import Qt.labs.platform 1.0
import "../fit.js" as Fit
import "../filemanager.js" as FileManager

Item {
    id: root
    Rectangle {
        id: base
        anchors.fill: listView
        radius: Fit.fit(5)
        visible: false
        color: "#d0d4d7"
        border.color: "#c2c6c9"
    }
    DropShadow {
        anchors.fill: base
        horizontalOffset: 0
        verticalOffset: Fit.fit(1)
        radius: Fit.fit(7.0)
        samples: Fit.fit(25)
        color: "#20000000"
        source: base
    }
    Rectangle {
        id: mask
        anchors.fill: listView
        radius: Fit.fit(5)
        visible: false
    }
    ListView {
        id: listView
        anchors.horizontalCenter: parent.horizontalCenter
        width: Fit.fit(300)
        clip: true
        height: parent.height - btnBuildOutCont.height - 2 * Fit.fit(15)
        delegate: projectListDelegate
        model: arches.length
        highlight: highlight
        highlightMoveDuration: 100
        highlightResizeDuration: 100
        ScrollBar.vertical: ScrollBar { /*contentItem.opacity: 1; onActiveChanged: active = false;*/ }
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: mask
        }
        onCurrentItemChanged: {
            currentBuildLabel = buildLabels[listView.currentIndex]
        }
    }
    Item {
        id: btnBuildOutCont
        width: Fit.fit(200)
        height: Fit.fit(28)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: Fit.fit(12)
        anchors.top: listView.bottom
        Item {
            id: btnBuildInCont
            anchors.fill: parent
            visible: false
            Rectangle {
                id: btnBuild
                anchors.fill: parent
                visible: false
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#F4BA48" }
                    GradientStop { position: 1.0; color: Qt.darker("#F4BA48", 1.1) }
                }
                Row {
                    anchors.centerIn: parent
                    spacing: Fit.fit(6)
                    Text {
                        text: "Build"
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        height: btnBuildImg.height
                        font.pixelSize: Fit.fit(13)
                    }
                    Image {
                        id: btnBuildImg
                        source: "qrc:///resources/images/ok.png"
                        height: btnBuild.height - Fit.fit(14)
                        fillMode: Image.PreserveAspectFit
                    }
                }
                signal clicked();
                function glow() {
                    btnBuildOverlay.color = "#30000000"
                }
                function unglow() {
                    btnBuildOverlay.color = "#00ffffff"
                }
            }
            ColorOverlay {
                id: btnBuildOverlay
                anchors.fill: btnBuild
                source: btnBuild
                color: "#00ffffff"
            }
        }
        Rectangle {
            id: btnBuildMask;
            anchors.fill: parent;
            radius: Fit.fit(6);
            visible: false;
        }
        OpacityMask {
            id: btnBuildOpMask
            visible: false
            anchors.fill: btnBuildInCont
            source: btnBuildInCont
            maskSource: btnBuildMask
        }
        DropShadow {
            anchors.fill: btnBuildOpMask
            horizontalOffset: 0
            verticalOffset: Fit.fit(1)
            radius: Fit.fit(6.0)
            samples: Fit.fit(14)
            color: "#80000000"
            source: btnBuildOpMask
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: btnBuild.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onPressed: btnBuild.glow()
            onReleased: btnBuild.unglow()
            onClicked: btnBuildClicked()
        }
    }
    Component {
        id: projectListDelegate
        Item {
            height: Fit.fit(52)
            width: parent.width
            Row {
                anchors.fill: parent
                anchors.margins: Fit.fit(7)
                spacing: Fit.fit(6)
                Image {
                    source: platformIcons[index]
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        width: Fit.fit(320)
                        elide: Text.ElideRight
                        color: "#2e2a21"
                        text: platforms[index]
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                    }
                    Text {
                        width: Fit.fit(320)
                        elide: Text.ElideRight
                        color: Qt.lighter("#2e2a21", 1.2)
                        text: arches[index]
                        font.pixelSize: Fit.fit(12)
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                enabled: enabledBuilds[index]
                onClicked: {
                    listView.currentIndex = index
                }
            }
            Image {
                id: warnIcon
                source: "qrc:///resources/images/sign_warning.png"
                width: Fit.fit(12)
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Fit.fit(10)
                visible: false
            }
            DropShadow {
                anchors.fill: warnIcon
                horizontalOffset: 0
                verticalOffset: 0
                radius: Fit.fit(7)
                samples: Fit.fit(26)
                color: "#85000000"
                source: warnIcon
                visible: !enabledBuilds[index]
            }
        }
    }
    Component {
        id: highlight
        Rectangle {
            width: listView.width
            height: Fit.fit(48)
            color: "#10000000"
            border.color: "#15000000"
        }
    }
    signal btnBuildClicked()
    property string currentBuildLabel: ""
    property var enabledBuilds: [true, true, false, false, false, false, false, false, false]
    property var buildLabels: ["android-armeabi-v7a", "android-x86", "ios", "windows-x86", "windows-x64", "macos", "linux-x86", "linux-x64", "raspi"]
    property var platforms: ["Android 4.2+", "Android 4.2+", "IOS 10+", "Windows 7+", "Windows 7+", "macOS 10.10+", "Linux", "Linux", "Raspberry Pi"]
    property var arches: ["Architecture: armeabi-v7a", "Architecture: x86", "Architectures: armv7s, arm64", "Architecture: x86", "Architecture: x64", "Architecture: x64", "Architecture: x86", "Architecture: x64", "Version: Pi 2+"]
    property var platformIcons: ["qrc:///resources/images/android.png", "qrc:///resources/images/android.png", "qrc:///resources/images/ios.png", "qrc:///resources/images/windows.png", "qrc:///resources/images/windows.png",
        "qrc:///resources/images/macos.png", "qrc:///resources/images/linux.png", "qrc:///resources/images/linux.png", "qrc:///resources/images/raspi.png"]
}
