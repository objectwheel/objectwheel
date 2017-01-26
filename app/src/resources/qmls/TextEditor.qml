import QtQuick 2.7
import QtQuick.Controls 2.1
import com.objectwheel.components 1.0
import "fit.js" as Fit

Flickable {
    id: root
    enabled: opacity > 0.98 ? true : false
    flickableDirection: Flickable.VerticalFlick
    boundsBehavior: Flickable.DragOverBounds
    contentHeight: editor.paintedHeight
    clip: true
    ScrollBar.vertical: ScrollBar { }

    Column {
        id: lineNumber
        anchors { left: parent.left; top: parent.top }
        Repeater {
            id: lineNumberRepeater
            clip: true
            Text {
                property alias bgcolor: rect.color
                text: index + 1
                color: "#e0e0e0"
                width: maxWidth
                height: editor.cursorRectangle.height
                horizontalAlignment: TextEdit.AlignRight
                verticalAlignment: TextEdit.AlignVCenter
                font.bold: Qt.colorEqual(bgcolor, "#c74c3c") ? true : false
                Component.onCompleted: if (contentWidth > maxWidth) maxWidth = contentWidth
                Rectangle {
                    id: rect
                    color: 'transparent'
                    anchors.fill: parent
                    z:-1
                }
            }
        }
    }
    Rectangle {
        id: leftLine
        width: Fit.fit(1)
        height: parent.height
        anchors.top: parent.top
        anchors.left: lineNumber.right
        anchors.leftMargin: Fit.fit(5)
        color: Qt.lighter(container.color, 1.3)
    }
    Rectangle {
        id: editorCurrentLineHighlight
        anchors.left: lineNumber.left
        anchors.right: editor.right
        visible: editor.focus
        height: editor.cursorRectangle.height
        y: editor.cursorRectangle.y
        color: Qt.darker(container.color, 1.3)
        z: -1
    }
    TextEdit {
        id: editor
        objectName: "editor"
        anchors {left:lineNumber.right;right:parent.right;top:parent.top;bottom:parent.bottom;leftMargin:Fit.fit(15)}
        wrapMode: root.opacity > 0.98 ? TextEdit.WrapAtWordBoundaryOrAnywhere : TextEdit.NoWrap ;
        renderType: Text.NativeRendering
        onTextChanged: timer.restart();
        onSelectedTextChanged: {
            if (editor.selectionStart !== editor.selectionEnd) {
                navbar.state = 'selection'
            } else if (editor.selectedText === "") {
                navbar.state = 'view'
            }
        }

        onLineCountChanged: {
            lineNumberRepeater.model = editor.lineCount
        }

        color: '#e0e0e0'
        selectionColor: '#0C75BC'
        selectByMouse: true
        text: documentHandler.text
        inputMethodHints: Qt.ImhNoPredictiveText

        DocumentHandler {
            id: documentHandler
            target: editor
        }

        MouseArea {
            id: handler
            anchors.fill: parent
            anchors.rightMargin: Fit.fit(10)
            propagateComposedEvents: true
            cursorShape: Qt.IBeamCursor
            onPressed: {
                editor.cursorPosition = parent.positionAt(mouse.x, mouse.y);
                Qt.inputMethod.hide();
                if (isDesktop) {
                    mouse.accepted = false
                }
            }
            onPressAndHold: {
                navbar.state = 'selection'
                Qt.inputMethod.hide();
            }
            onDoubleClicked: {
                editor.focus = true
                Qt.inputMethod.show();
            }
        }
    }

    property var timer
    property var navbar
    property alias editor: editor
    property alias lineNumberRepeater: lineNumberRepeater
}
