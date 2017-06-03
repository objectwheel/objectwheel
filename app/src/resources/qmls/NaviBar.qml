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
        visible: barVisible
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0D74C8" }
            GradientStop { position: 1.0; color: Qt.darker("#0D74C8", 1.2) }
        }
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
                gradient: Gradient {
                    GradientStop { position: 0.0; color: splitState == 'editor' ? "white" : "transparent" }
                    GradientStop { position: 1.0; color: splitState == 'editor' ? Qt.darker("white", 1.1) : "transparent" }
                }
                Text {
                    text: "Editor"
                    color: splitState == 'editor' ? Qt.darker("#0D74C8", 1.1) : 'white'
                    anchors.centerIn: parent
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            Rectangle {
                id: seperator
                color: Qt.darker("#0D74C8", 1.3)
                Layout.maximumWidth: Fit.fit(1)
                Layout.minimumWidth: Fit.fit(1)
                Layout.fillHeight: true
                Layout.bottomMargin: Fit.fit(3)
                Layout.topMargin: Fit.fit(3)
            }
            Rectangle {
                id: second
                gradient: Gradient {
                    GradientStop { position: 0.0; color: splitState == 'splitted' ? "white" : "transparent" }
                    GradientStop { position: 1.0; color: splitState == 'splitted' ? Qt.darker("white", 1.1) : "transparent" }
                }
                Text {
                    text: "Split"
                    color: splitState == 'splitted' ? Qt.darker("#0D74C8", 1.1) : "white"
                    anchors.centerIn: parent
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
            Rectangle {
                id: seperator2
                color: Qt.darker("#0D74C8", 1.3)
                Layout.maximumWidth: Fit.fit(1)
                Layout.minimumWidth: Fit.fit(1)
                Layout.fillHeight: true
                Layout.bottomMargin: Fit.fit(3)
                Layout.topMargin: Fit.fit(3)
            }
            Rectangle {
                id: third
                gradient: Gradient {
                    GradientStop { position: 0.0; color: splitState == 'viewer' ? "white" : "transparent" }
                    GradientStop { position: 1.0; color: splitState == 'viewer' ? Qt.darker("white", 1.1) : "transparent" }
                }
                Text {
                    text: "Viewer"
                    color: splitState == 'viewer' ? Qt.darker("#0D74C8", 1.1) : "white"
                    anchors.centerIn: parent
                }
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }

    }

    Row {
        anchors.centerIn: parent
        visible: barVisible ? (parent.state === 'view') : false
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
        visible: barVisible ? (parent.state == 'selection') : false
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Fit.fit(10)
        }
        text: "⏏"
        color: "white"
        MouseArea {
            anchors.fill: parent
            anchors.margins: -Fit.fit(5)
            cursorShape: Qt.PointingHandCursor
            onPressed: {
                navigationBar.state = 'view';
                editor.editor.deselect();
            }
        }
    }

    Row {
        anchors.centerIn: parent
        visible: barVisible ? (parent.state === 'selection') : false
        spacing: Fit.fit(40)
        Text {
            visible: (editor.editor.selectionStart !== editor.editor.selectionEnd)
            color: "white"
            text: "Cut"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.editor.cut()
            }
        }
        Text {
            visible: (editor.editor.selectionStart !== editor.editor.selectionEnd)
            color: "white"
            text: "Copy"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: {
                    editor.editor.copy()
                    editor.editor.deselect()
                }
            }
        }
        Text {
            visible: (editor.editor.selectionStart === editor.editor.selectionEnd)
            color: "white"
            text: "Select"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.editor.selectWord()
            }
        }
        Text {
            visible: (editor.editor.selectionStart === editor.editor.selectionEnd)
            color: "white"
            text: "Select All"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.editor.selectAll()
            }
        }
        Text {
            visible: (editor.editor.canPaste === true)
            color: "white"
            text: "Paste"
            MouseArea {
                anchors.fill: parent
                anchors.margins: -Fit.fit(5)
                cursorShape: Qt.PointingHandCursor
                onPressed: editor.editor.paste()
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

    property bool barVisible: true
}
