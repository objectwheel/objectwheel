import QtQuick 2.7
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0

Item {

    Rectangle {
        id: base
        anchors.fill: parent
        radius: fit(5)
        visible: false
        color: "#d0d4d7"
        border.color: "#c2c6c9"
    }

    DropShadow {
        anchors.fill: base
        horizontalOffset: 0
        verticalOffset: fit(1)
        radius: fit(7.0)
        samples: fit(25)
        color: "#20000000"
        source: base
    }

    Rectangle {
        id: mask
        anchors.fill: listView
        radius: fit(5)
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
            height: fit(48)
            color: "#10000000"
            border.color: "#15000000"
        }
    }

    Component {
        id: projectListDelegate
        Item {
            height: fit(48)
            width: parent.width
            Row {
                anchors.fill: parent
                anchors.margins: fit(4)
                spacing: fit(5)
                Image {
                    source: "images/fileicon.png"
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        width: fit(320)
                        elide: Text.ElideRight
                        color: "#2e2a21"
                        text: projectName
                        font.bold: true
                        font.pixelSize: fit(13)
                    }
                    Text {
                        width: fit(320)
                        elide: Text.ElideRight
                        color: Qt.lighter("#2e2a21", 1.2)
                        text: lastEdited
                        font.pixelSize: fit(12)
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
                source: "images/info.png"
                height: Math.floor(parent.height / 2.5)
                fillMode: Image.PreserveAspectFit
                anchors {verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: fit(5) }
                visible: listView.currentItem == parent
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        infoClicked(projectName)
                    }
                }
            }

        }
    }
    signal infoClicked(var projectName)
    property string selectedProjectname
    property alias listView : listView
    function fit(val) { return val }
}
