import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import com.objectwheel.components 1.0

Item {
    id: root
    signal saved(string code)

    property real scaleRatio: 1
    property string splitState: 'splitted'
    property int lineNumberPadding: 20 * scaleRatio
    property int lineNumberSpacing: 0
    property var previousItem: null
    property int errorLineNumber: 0

    Rectangle {
        id:toolBar
        z: 2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 32 * scaleRatio
        color: "#666666"

        RowLayout {
            anchors.margins: 6 * scaleRatio
            anchors.fill: parent
            spacing: 3 * scaleRatio

            Image {
                id: undo
                height: 10
                width: height
                sourceSize.width: width * 2.0
                sourceSize.height: height
                Layout.fillHeight: true
                source: "qrc:///resources/images/left-arrow.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: editor.undo()
                }
            }

            Item { Layout.fillWidth: true }

            Image {
                id: save
                height: 10
                width: height
                sourceSize.width: width
                sourceSize.height: height
                Layout.fillHeight: true
                source: "qrc:///resources/images/save-icon.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.saved(editor.text)
                }
            }

            Item { Layout.fillWidth: true }

            Image {
                id: redo
                height: 10
                width: height
                sourceSize.width: width * 2.0
                sourceSize.height: height
                Layout.fillHeight: true
                source: "qrc:///resources/images/right-arrow.png"
                MouseArea {
                    anchors.fill: parent
                    onClicked: editor.redo()
                }
            }
        }

        Image {
            fillMode: Image.TileHorizontally
            source: "qrc:///resources/images/shadow.png"
            width: toolBar.width
            anchors.top: toolBar.bottom
            height: 4 * scaleRatio
            z:2
            opacity: 0.5
        }
    }

    Item  {
        id: editorContainer
        anchors{left:parent.left;right:parent.right;top:toolBar.bottom;bottom: parent.bottom}

        Timer {
            id: timer
            interval: 500;
            onTriggered: reloadView()
        }

        NaviBar {
            state: "view"
            id: navibar
            z: 2
        }

        Item {
            id: view
            state: root.splitState
            width: root.width/2
            height: root.height
            anchors { top: parent.top; right: parent.right; bottom: parent.bottom; }
            visible: opacity > 0 ? true : false

            Rectangle {
                id: viewContent
                anchors.fill: parent
                color:"#475351"
            }

            states: [
                State {
                    name: "splitted"
                    PropertyChanges { target: view; width: root.width/2 }
                    PropertyChanges { target: view; opacity: 1 }
                    PropertyChanges { target: background; width: root.width/2 }
                    PropertyChanges { target: background; opacity: 1 }
                },
                State {
                    name: "editor"
                    PropertyChanges { target: view; width: 0 }
                    PropertyChanges { target: background; width: root.width }
                    PropertyChanges { target: background; opacity: 1 }
                },
                State {
                    name: "viewer"
                    PropertyChanges { target: view; width: root.width }
                    PropertyChanges { target: view; opacity: 1 }
                    PropertyChanges { target: background; width: 0 }
                }
            ]
            transitions: [
                Transition {
                    to: "*"
                    NumberAnimation { target: view; properties: "width"; duration: 300; easing.type: Easing.InOutQuad; }
                    NumberAnimation { target: background; properties: "width"; duration: 300; easing.type: Easing.InOutQuad; }
                }
            ]
        }

        Rectangle {
            id: errorHead
            color: "#007edf"
            z:2
            anchors.horizontalCenter: background.horizontalCenter
            width: background.width
            height: 40 * scaleRatio
            y: -height

            Behavior on y {
                NumberAnimation { duration: 500; easing.type: Easing.OutExpo }
            }

            Text {
                id: errorMessage
                anchors { top: parent.top; left: parent.left; right: parent.right; bottom: parent.bottom }
                wrapMode: Text.WordWrap
                text: ""
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Component.onCompleted: {
                errorMessage.textChanged.connect(handleError)
            }

            function handleError() {
                if (errorMessage.text != "") {
                    y = -0 * scaleRatio
                } else {
                    y = -height
                }
            }

            Text {
                anchors {right: parent.right; bottom: parent.bottom; margins:5 * scaleRatio}
                width: 10 * scaleRatio
                height: width
                text: "⏏"
                color: "white"
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        errorMessage.text = ""
                    }
                }
            }
        }

        Rectangle {
            id: background
            width: root.width/2
            height: root.height
            anchors { top: errorHead.bottom; left: parent.left; bottom: parent.bottom;}
            color: '#333333'
            visible: opacity > 0 ? true : false

            Flickable {
                id: flickable
                anchors.fill: parent;
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.DragOverBounds
                contentWidth: parent.width
                contentHeight: editor.height + 40 * scaleRatio
                clip: true

                Column {
                    id: lineNumber
                    anchors { margins: 20 * scaleRatio; left: parent.left; top: parent.top }
                    spacing: lineNumberSpacing
                    Repeater {
                        id: lineNumberRepeater
                        model: editor.lineCount
                        Text {
                            property alias bgcolor: rect.color
                            width: 20 * scaleRatio
                            text: index + 1
                            height: editor.cursorRectangle.height
                            color: 'lightgray'
                            horizontalAlignment: TextEdit.AlignHCenter
                            Rectangle {
                                id: rect
                                color: 'transparent'
                                anchors.fill: parent
                                opacity: 0.5
                            }
                        }
                    }
                }

                Rectangle {
                    id: editorCurrentLineHighlight
                    anchors {
                        left: lineNumber.right
                        margins: lineNumberPadding
                    }
                    visible: editor.focus
                    width: editor.width
                    height: editor.cursorRectangle.height
                    y: editor.cursorRectangle.y + lineNumberPadding
                    color: '#454545'
                }

                TextEdit {
                    id: editor
                    objectName: "editor"
                    anchors {
                        margins: lineNumberPadding
                        left: lineNumber.right; right: parent.right; top: parent.top;
                    }
                    wrapMode: TextEdit.WrapAtWordBoundaryOrAnywhere;
                    renderType: Text.NativeRendering
                    onTextChanged: timer.restart();

                    onSelectedTextChanged: {
                        if (editor.selectedText === "") {
                            navibar.state = 'view'
                        }
                    }
                    // FIXME: stupid workaround for indent
                    Keys.onPressed: {
                        if (event.key == Qt.Key_BraceRight) {
                            editor.select(0, cursorPosition)
                            var previousContent = editor.selectedText.split(/\r\n|\r|\n/)
                            editor.deselect()
                            var currentLine = previousContent[previousContent.length - 1]
                            var leftBrace = /{/, rightBrace = /}/;
                            if (!leftBrace.test(currentLine)) {
                                editor.remove(cursorPosition, cursorPosition - currentLine.length);
                                currentLine = currentLine.toString().replace(/ {1,4}$/, "");
                                editor.insert(cursorPosition, currentLine);
                            }
                        }
                    }
                    Keys.onReturnPressed: {
                        editor.select(0, cursorPosition)
                        var previousContent = editor.selectedText.split(/\r\n|\r|\n/)
                        editor.deselect()
                        var currentLine = previousContent[previousContent.length - 1]
                        var leftBrace = /{/, rightBrace = /}/;
                        editor.insert(cursorPosition, "\n")
                        var whitespaceAppend = currentLine.match(new RegExp(/^[ \t]*/))  // whitespace
                        if (leftBrace.test(currentLine)) // indent
                            whitespaceAppend += "    ";
                        editor.insert(cursorPosition, whitespaceAppend)
                    }

                    color: '#c5c8c6'
                    selectionColor: '#0C75BC'
                    selectByMouse: true
                    text: documentHandler.text
                    inputMethodHints: Qt.ImhNoPredictiveText

                    DocumentHandler {
                        id: documentHandler
                        target: editor
                        Component.onCompleted: {
                            documentHandler.text = "import QtQuick 2.0\n\nRectangle { \n    color: '#FEEB75'" +
                                    "\n    Text { \n        anchors.centerIn: parent" +
                                    "\n        text: 'Hello, World!' \n\t\tcolor: \"white\"; \n    } \n}"
                        }
                    }

                    // FIXME: add selection / copy / paste popup
                    MouseArea {
                        id: handler
                        enabled: !isDesktop
                        anchors.fill: parent
                        propagateComposedEvents: true
                        onPressed: {
                            editor.cursorPosition = parent.positionAt(mouse.x, mouse.y);
                            editor.focus = true
                            navibar.state = 'view'
                            Qt.inputMethod.show();
                        }
                        onPressAndHold: {
                            navibar.state = 'selection'
                            Qt.inputMethod.hide();
                        }
                        onDoubleClicked: {
                            editor.selectWord()
                            navibar.state = 'selection'
                        }
                    }
                }
            }
        }
    }

    function reloadView() {
        if (previousItem != null) {
            previousItem.destroy()
        }
        try {
            var newItem = Qt.createQmlObject(editor.text, viewContent);
        } catch (e) {
            previousItem = null;
            var error = e.qmlErrors[0];
            errorLineNumber = error.lineNumber;
            lineNumberRepeater.itemAt(errorLineNumber - 1).bgcolor = 'red'
            errorMessage.text = "Error, line " + error.lineNumber + ":" +
                    error.columnNumber + " : " + error.message;
            return;
        }
        if (errorLineNumber > 0) {
            lineNumberRepeater.itemAt(errorLineNumber - 1).bgcolor = 'transparent'
            errorLineNumber = 0;
            errorMessage.text = ""
        }
        newItem.anchors.centerIn = viewContent
        previousItem = newItem;
    }
}
