import QtQuick 2.7
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import "../fit.js" as Fit

Item {

    Rectangle {
        id: base
        anchors.fill: parent
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
        anchors.fill: parent
        width: parent.width
        clip: true
        height: parent.height
        delegate: projectListDelegate
        highlight: highlight
        highlightMoveDuration: 100
        highlightResizeDuration: 100
        ScrollBar.vertical: ScrollBar { }
        layer.enabled: true
        layer.effect: OpacityMask{
            maskSource: mask
        }
        onCurrentItemChanged: {
            selectedProjectname = model.get(currentIndex).projectName
        }
    }
    Text {
        anchors.centerIn: parent
        text: "No Project"
        color: "#909497"
        visible: listView.count < 1
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
    Component {
        id: projectListDelegate
        Item {
            height: Fit.fit(48)
            width: parent.width
            Row {
                anchors.fill: parent
                anchors.margins: Fit.fit(4)
                spacing: Fit.fit(5)
                Image {
                    source: "qrc:///resources/images/fileicon.png"
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter

                    RadialGradient {
                        id: activeGradient
                        width: Fit.fit(10)
                        height: width
                        visible: false
                        anchors { left: parent.left; top: parent.top; topMargin: Fit.fit(-0.5); leftMargin: Fit.fit(3) }
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#6bcb36" }
                            GradientStop { position: 0.5; color: "#4db025" }
                        }
                    }
                    Rectangle {
                        id: mask
                        visible: false
                        radius: activeGradient.width / 2.0
                        anchors.fill: activeGradient
                    }
                    OpacityMask {
                        id: opMask
                        source: activeGradient
                        anchors.fill: activeGradient
                        maskSource: mask
                        visible: false
                    }
                    DropShadow {
                        anchors.fill: opMask
                        horizontalOffset: 0
                        verticalOffset: Fit.fit(1)
                        radius: Fit.fit(6.0)
                        samples: Fit.fit(25)
                        color: "#70000000"
                        source: opMask
                        visible: active
                    }
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        width: Fit.fit(320)
                        elide: Text.ElideRight
                        color: "#2e2a21"
                        text: projectName
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                    }
                    Text {
                        width: Fit.fit(320)
                        elide: Text.ElideRight
                        color: Qt.lighter("#2e2a21", 1.2)
                        text: lastEdited
                        font.pixelSize: Fit.fit(12)
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index
                }
            }
            Image {
                id: infImage
                source: "qrc:///resources/images/info.png"
                height: Math.floor(parent.height / 2.5)
                fillMode: Image.PreserveAspectFit
                anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: Fit.fit(5) }
                visible: false
            }
            ColorOverlay {
                id: infColorOverlay
                source: infImage
                anchors.fill: infImage
                color: "#01000000"
                visible: listView.currentItem == parent
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onPressed: infColorOverlay.color = "#50000000"
                    onReleased: infColorOverlay.color = "#01000000"
                    onClicked: {
                        infoClicked(projectName)
                    }
                }
            }
        }
    }
    signal infoClicked(var projectName)
    property string selectedProjectname
    property alias listView: listView
}
