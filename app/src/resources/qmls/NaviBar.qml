import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    id: navigationBar
    width: parent.width
    height: 44 * scaleRatio
    anchors {
        bottom: parent.bottom
        left: parent.left
    }
    color: "transparent"

    Rectangle {
        id: repeater
        border.color: "#007edf"
        width: 240 * scaleRatio
        height: 29 * scaleRatio
        anchors.centerIn: parent
        radius: 5 * scaleRatio
        visible: false

        Row {
            Rectangle {
                width: 80 * scaleRatio ; height: 29 * scaleRatio
                color: splitState == 'editor' ? "#007edf" : "transparent"
                Text {
                    text: "Editor"
                    color: splitState == 'editor' ? "white" : '#007edf'
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: 80 * scaleRatio; height: 29 * scaleRatio
                border.color: "#007edf"
                color: splitState == 'splitted' ? "#007edf" : "transparent"
                Text {
                    text: "Split"
                    color: splitState == 'splitted' ? "white" : "#007edf"
                    anchors.centerIn: parent
                }
            }
            Rectangle {
                width: 80 * scaleRatio; height: 29 * scaleRatio
                color: splitState == 'viewer' ? "#007edf" : "transparent"
                Text {
                    text: "Viewer"
                    color: splitState == 'viewer' ? "white" : "#007edf"
                    anchors.centerIn: parent
                }
            }
        }

    }
    Rectangle {
        id: mask
        width: repeater.width
        height: repeater.height
        anchors.fill: repeater
        radius: 5 * scaleRatio
    }
    OpacityMask {
        visible: (parent.state === 'view')
        anchors.fill: repeater
        source: repeater
        maskSource: mask
    }

    Row {
        anchors.centerIn: parent
        visible: (parent.state === 'view')
        MouseArea {
            width: 80 * scaleRatio
            height: 29 * scaleRatio
            onPressed: splitState = 'editor'
        }
        MouseArea {
            width: 80 * scaleRatio
            height: 29 * scaleRatio
            onPressed: splitState = 'splitted'
        }
        MouseArea {
            width: 80 * scaleRatio
            height: 29 * scaleRatio
            onPressed: splitState = 'viewer'
        }
    }

    Text {
        visible: (parent.state == 'selection')
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            margins: 10 * scaleRatio
        }
        text: "⏏"
        color: 'grey'
        MouseArea {
            anchors.fill: parent
            anchors.margins: -5 * scaleRatio
            onPressed: {
                navigationBar.state = 'view';
                editor.deselect();
            }
        }
    }

    Row {
        anchors.centerIn: parent
        visible: (parent.state === 'selection')
        spacing: 20 * scaleRatio
        Text {
            visible: (editor.selectionStart !== editor.selectionEnd)
            color: "#007edf"
            text: "Cut"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 * scaleRatio
                onPressed: editor.cut()
            }
        }
        Text {
            visible: (editor.selectionStart !== editor.selectionEnd)
            color: "#007edf"
            text: "Copy"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 * scaleRatio
                onPressed: {
                    editor.copy()
                    editor.deselect()
                }
            }
        }
        Text {
            visible: (editor.selectionStart === editor.selectionEnd)
            color: "#007edf"
            text: "Select"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 * scaleRatio
                onPressed: editor.selectWord()
            }
        }
        Text {
            visible: (editor.selectionStart === editor.selectionEnd)
            color: "#007edf"
            text: "Select All"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 * scaleRatio
                onPressed: editor.selectAll()
            }
        }
        Text {
            visible: (editor.canPaste === true)
            color: "#007edf"
            text: "Paste"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 * scaleRatio
                onPressed: editor.paste()
            }
        }
    }

    states: [
        State {
            name: "view"
        },
        State {
            name: "selection"
        }
    ]
}
