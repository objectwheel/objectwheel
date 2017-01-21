import QtQuick 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import "fit.js" as Fit

Item {
    id: navigationBar

    Rectangle {
        id: mask
        anchors.fill: repeater
        radius: Fit.fit(5)
        visible: false
    }

    Rectangle {
        id: repeater
        width: Fit.fit(240)
        height: Fit.fit(29)
        anchors.centerIn: parent
        visible: true
        color: "#2b5796"
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: mask
        }
        RowLayout {
            id: viewRow
            anchors.fill: parent
            spacing: 0
            Rectangle {
                id: first
                color: splitState == 'editor' ? "white" : "transparent"
                Text {
                    text: "Editor"
                    color: splitState == 'editor' ? "#2b5796" : 'white'
                    anchors.centerIn: parent
                    font.bold: true
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            Rectangle {
                id: seperator
                color: Qt.lighter("#2b5796", 1.12)
                Layout.maximumWidth: Fit.fit(1)
                Layout.minimumWidth: Fit.fit(1)
                Layout.fillHeight: true
                Layout.bottomMargin: Fit.fit(3)
                Layout.topMargin: Fit.fit(3)
            }
            Rectangle {
                id: second
                color: splitState == 'splitted' ? "white" : "transparent"
                Text {
                    text: "Split"
                    color: splitState == 'splitted' ? "#2b5796" : "white"
                    anchors.centerIn: parent
                    font.bold: true
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            Rectangle {
                id: seperator2
                color: Qt.lighter("#2b5796", 1.12)
                Layout.maximumWidth: Fit.fit(1)
                Layout.minimumWidth: Fit.fit(1)
                Layout.fillHeight: true
                Layout.bottomMargin: Fit.fit(3)
                Layout.topMargin: Fit.fit(3)
            }
            Rectangle {
                id: third
                color: splitState == 'viewer' ? "white" : "transparent"
                Text {
                    text: "Viewer"
                    color: splitState == 'viewer' ? "#2b5796" : "white"
                    anchors.centerIn: parent
                    font.bold: true
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }

    }

    Row {
        anchors.centerIn: parent
        visible: (parent.state === 'view')
        MouseArea {
            width: Fit.fit(80)
            height: Fit.fit(29)
            cursorShape: Qt.PointingHandCursor
            onPressed: splitState = 'editor'
        }
        MouseArea {
            width: Fit.fit(80)
            height: Fit.fit(29)
            cursorShape: Qt.PointingHandCursor
            onPressed: splitState = 'splitted'
        }
        MouseArea {
            width: Fit.fit(80)
            height: Fit.fit(29)
            cursorShape: Qt.PointingHandCursor
            onPressed: splitState = 'viewer'
        }
    }

    Text {
        visible: (parent.state == 'selection')
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Fit.fit(10)
        }
        text: "⏏"
        color: "white"
        font.bold: true
        MouseArea {
            anchors.fill: parent
            anchors.margins: -Fit.fit(5)
            cursorShape: Qt.PointingHandCursor
            onPressed: {
                navigationBar.state = 'view';
                editor.deselect();
            }
        }
    }

    Row {
        anchors.centerIn: parent
        visible: (parent.state === 'selection')
        spacing: Fit.fit(30)
        Text {
            visible: (editor.selectionStart !== editor.selectionEnd)
            color: "white"
            text: "Cut"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.cut()
            }
        }
        Text {
            visible: (editor.selectionStart !== editor.selectionEnd)
            color: "white"
            text: "Copy"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: {
                    editor.copy()
                    editor.deselect()
                }
            }
        }
        Text {
            visible: (editor.selectionStart === editor.selectionEnd)
            color: "white"
            text: "Select"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.selectWord()
            }
        }
        Text {
            visible: (editor.selectionStart === editor.selectionEnd)
            color: "white"
            text: "Select All"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.selectAll()
            }
        }
        Text {
            visible: (editor.canPaste === true)
            color: "white"
            text: "Paste"
            font.bold: true
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.paste()
            }
        }
    }

    onStateChanged: {
        viewRow.visible = (state !== 'selection')
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
