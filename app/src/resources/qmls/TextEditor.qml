import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import com.objectwheel.components 1.0
import "fit.js" as Fit

Flickable {
    id: root
    enabled: opacity > 0.98 ? true : false
    flickableDirection: Flickable.VerticalFlick
    boundsBehavior: Flickable.DragOverBounds
    contentHeight: editor.contentHeight
    clip: true
    ScrollBar.vertical: ScrollBar { }
    Column {
        id: lineNumber
        anchors { left: parent.left; top: parent.top }
        spacing: 0
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
    TextArea {
        id: editor
        objectName: "editor"
        frameVisible: false
        backgroundVisible: false
        anchors {left:lineNumber.right;right:parent.right;top:parent.top;bottom:parent.bottom;leftMargin:Fit.fit(15);rightMargin: Fit.fit(10)}
        wrapMode: root.opacity > 0.98 ? TextEdit.WrapAtWordBoundaryOrAnywhere : TextEdit.NoWrap ;
        selectByMouse: true
        inputMethodHints: Qt.ImhNoPredictiveText
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        onTextChanged: root.textChanged(text)
        style: TextAreaStyle {
            renderType: Text.NativeRendering
            textColor: "#e0e0e0"
            selectionColor: "#0C75BC"
            textMargin: 0
        }
        onSelectedTextChanged: {
            if (editor.selectedText === "") {
                navbar.state = 'view'
            }
        }
        onLineCountChanged: {
            lineNumberRepeater.model = editor.lineCount
        }
        DocumentHandler {
            id: documentHandler
            target: editor
        }
    }
    signal textChanged(var text)
    property var navbar
    property var tmr: null
    property alias editor: editor
    property alias lineNumberRepeater: lineNumberRepeater
}
