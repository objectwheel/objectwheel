import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import com.objectwheel.components 1.0
import com.objectwheel.editor 1.0
import "fit.js" as Fit
import "filemanager.js" as FileManager
import "delaycaller.js" as DelayCaller

//TODO: Fix qml editor to start with a default main.qml and default root directory
//TODO: Fix keyboard problems of qml editor
//TODO: Fix popup window closing method
//TODO: Fix android logo
//TODO: Add IOS/MacOS/Linux/Windows logo on each platform
//TODO: Fix objectwheel's to capital "Objectwheel" everywhere
//TODO: Fix warnings on Visual Studio
//TODO: Add custom media donwloader to folder browser
//TODO: Add "custom control add" buttons to toolbox
//TODO: Fix qml file names in resource
//TODO: Fix bugs on binding editor
//TODO: Fix popup window colors
//TODO: Add "Fit" lib to com.objectwheel.components
//TODO: Save doesn't work
//TODO: Update FileManager.js funcs
//TODO: Show parent folder in folderList's upside
//TODO: Open editor only for texts
//TODO: Keşe alınan dosya silinirse ?
//TODO: Keşe alınan dosyanın adı değişirse ?
//FIX: indexOf(toolDir) bunları fixle

Item {
    id: root

    FolderBrowser {
        id: folderBrowser
        clip: true
        anchors { top: parent.top; bottom: parent.bottom; }
        width: Fit.fit(170)
        x: menu.checked ? 0 : -width
        enabled: toolboxMode
        Component.onCompleted: anim.enabled = true
        onEntryEdited: {
            from = from.toString().replace("file://", "")
            to = to.toString().replace("file://", "")
            if (isdir) {
                console.log(from + "," + to)
            } else if (isTextFile(from) && isTextFile(to)) {
                for (var i = 0; i < urlCache.length; i++) {
                    if (urlCache[i] === from) {
                        urlCache[i] = to
                    }
                }
            } else if (isTextFile(from) && !isTextFile(to)) {
                for (var ii = 0; ii < urlCache.length; ii++) {
                    if (urlCache[ii] === from) {
                        FileManager.svfile(to, saveCache[ii])
                        urlCache.splice(ii, 1)
                        saveCache.splice(ii, 1)

                        for (var j=0; j<folderBrowser.count;j++) {
                            if (folderBrowser.itemAt(j) === "main.qml") {
                                folderBrowser.currentIndex = j
                            }
                        }
                    }
                }
            } else if (!isTextFile(from) && isTextFile(to)) {
                root.url = to
            }

            reloadView()
        }
        onEntryRemoved: {
            urlval = urlval.toString().replace("file://", "")
            if (!isdir && isTextFile(urlval)) {
                root.url = urlval
            }
        }

        onSelectionChanged:{
            urlval = urlval.toString().replace("file://", "")
            if (!isdir && isTextFile(urlval)) {
                root.url = urlval
            }
        }

        Behavior on x {
            id: anim
            enabled: false
            NumberAnimation {
                duration: 400
                easing.type: Easing.OutExpo
            }
        }
    }

    Item {
        id: container
        anchors { left:folderBrowser.right; top:parent.top; bottom: parent.bottom; right: parent.right }
        clip: true

        Rectangle {
            id:toolBar
            z: 2
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: Fit.fit(42)
            color: "#2b5796"
            clip: true

            RowLayout {
                anchors.margins: Fit.fit(6)
                anchors.fill: parent
                spacing: Fit.fit(3)

                FancyButton {
                    id: menu
                    width: height
                    checkable: true
                    height: parent.height
                    iconSource: "qrc:///resources/images/menu-icon.png"
                }

                Item { Layout.fillWidth: true }

                FancyButton {
                    id: save
                    width: height
                    enabled: ((errorMessage.text=="") && (editor.text!=""))
                    height: parent.height
                    iconSource: "qrc:///resources/images/save-icon.png"
                    onClicked: (root.url === "") ? root.saved(editor.text) : root.savedUrl(editor.text)
                }

                FancyButton {
                    id: undo
                    width: height
                    height: parent.height
                    enabled: editor.canUndo
                    iconSource: "qrc:///resources/images/left-arrow.png"
                    onClicked: editor.undo()
                }

                FancyButton {
                    id: redo
                    width: height
                    height: parent.height
                    enabled: editor.canRedo
                    iconSource: "qrc:///resources/images/right-arrow.png"
                    onClicked: editor.redo()
                }
            }

            Text {
                anchors.fill: parent
                text: "Objectwheel QML Editor"
                font.bold: true
                color: "white"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Rectangle  {
            id: editorContainer
            anchors{left:parent.left;right:parent.right;top:toolBar.bottom;bottom: parent.bottom}
            color: "#44504e"
            clip: true

            Item {
                id: view
                state: root.splitState
                anchors { top: errorHead.bottom; right: parent.right; bottom: parent.bottom; left: seperator.right}
                enabled: opacity > 0.98 ? true : false
                clip: true
                states: [
                    State {
                        name: "splitted"
                        PropertyChanges { target: view; opacity: 1 }
                        PropertyChanges { target: background; opacity: 1 }
                        PropertyChanges { target: handle; opacity: 1 }
                        PropertyChanges { target: seperator; x:parent.width/2.0 }
                    },
                    State {
                        name: "editor"
                        PropertyChanges { target: view; opacity: 0}
                        PropertyChanges { target: background; opacity: 1 }
                        PropertyChanges { target: handle; opacity: 0 }
                        PropertyChanges { target: seperator; x:parent.width }
                    },
                    State {
                        name: "viewer"
                        PropertyChanges { target: view; opacity: 1 }
                        PropertyChanges { target: background; opacity: 0}
                        PropertyChanges { target: handle; opacity: 0 }
                        PropertyChanges { target: seperator; x:Fit.fit(-1) }
                    }
                ]
                transitions: [
                    Transition {
                        to: "*"
                        NumberAnimation { target: seperator; properties: "x"; duration: 300; easing.type: Easing.InOutQuad; }
                        NumberAnimation { target: view; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                        NumberAnimation { target: background; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                        NumberAnimation { target: handle; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                    }
                ]
            }
            Item {
                id: background
                anchors { top: parent.top; left: parent.left; bottom: parent.bottom; right:seperator.left}
                enabled: opacity > 0.98 ? true : false
                clip: true
                Flickable {
                    id: flickable
                    anchors.fill: parent;
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.DragOverBounds
                    contentWidth: parent.width
                    contentHeight: editor.height + toolBar.height
                    clip: true
                    ScrollBar.vertical: ScrollBar { }

                    Column {
                        id: lineNumber
                        anchors { margins: Fit.fit(20); left: parent.left; top: parent.top }
                        spacing: lineNumberSpacing
                        Repeater {
                            id: lineNumberRepeater
                            model: editor.lineCount
                            Text {
                                property alias bgcolor: rect.color
                                width: Fit.fit(20)
                                text: index + 1
                                height: editor.cursorRectangle.height
                                color: "#e0e0e0"
                                horizontalAlignment: TextEdit.AlignHCenter
                                Rectangle {
                                    id: rect
                                    color: 'transparent'
                                    anchors.fill: parent
                                    opacity: 0.5
                                    z:-1
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
                        color: Qt.darker(editorContainer.color, 1.2)
                    }

                    TextEdit {
                        id: editor
                        objectName: "editor"
                        anchors {
                            margins: lineNumberPadding
                            left: lineNumber.right; right: parent.right; top: parent.top;
                        }
                        wrapMode: background.opacity > 0.98 ? TextEdit.WrapAtWordBoundaryOrAnywhere : TextEdit.NoWrap ;
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
            Rectangle {
                id: seperator
                x: parent.width/2.0
                color: Qt.lighter(parent.color, 1.5)
                width: Fit.fit(1)
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Fit.fit(50)
                anchors.topMargin: Fit.fit(10)

                MouseArea {
                    id: handle
                    height: Fit.fit(22)
                    width: height
                    anchors.centerIn: parent
                    drag.target: seperator;
                    drag.axis: "XAxis"
                    drag.minimumX: 0
                    drag.maximumX: container.width
                    drag.filterChildren: true
                    drag.smoothed: false
                    cursorShape: enabled ? Qt.SplitHCursor : Qt.ArrowCursor
                    enabled: opacity > 0.8
                    Image {
                        id: handleImage
                        anchors.fill: parent
                        sourceSize: Qt.size(width,height)
                        source: "qrc:///resources/images/handle.png"
                    }
                }

            }
            Rectangle {
                id: errorHead
                clip: true
                color: "#1e8145"
                anchors.horizontalCenter: view.horizontalCenter
                width: view.width
                height: Fit.fit(40)
                y: -height
                visible: 0.98 < Math.abs(y/height) ? false : true
                Behavior on y {
                    NumberAnimation { duration: 500; easing.type: Easing.OutExpo }
                }

                Text {
                    id: errorMessage
                    anchors { top: parent.top; left: parent.left; right: parent.right; bottom: parent.bottom }
                    wrapMode: Text.WordWrap
                    text: ""
                    color: "white"
                    clip: true
                    font.pixelSize: Fit.fit(11)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Component.onCompleted: {
                    errorMessage.textChanged.connect(handleError)
                }

                function handleError() {
                    if (errorMessage.text != "") {
                        y = 0
                    } else {
                        y = -height
                    }
                }

                Text {
                    anchors {right: parent.right; bottom: parent.bottom; margins:Fit.fit(5)}
                    text: "⏏"
                    color: "white"
                    font.bold: true
                    clip: true
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            errorHead.y = -errorHead.height
                        }
                    }
                }
            }
            NaviBar {
                state: "view"
                id: navibar
            }
            Timer {
                id: timer
                interval: 500;
                onTriggered: reloadView()
            }
        }
    }

    ShadowFactory {
        targets: [toolBar, container, navibar, handleImage]
        places: [Item.Bottom, Item.Left, Item.Bottom, Item.Bottom]
    }

    CacheCleaner { id: cacheCleaner }

    onUrlChanged: {
        var index
        var cacheFound = false
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i] === root.url) {
                cacheFound = true
                index = i
                break
            }
        }
        if (cacheFound) {
            editor.text = saveCache[index]
        } else {
            urlCache.push(root.url)
            saveCache.push(FileManager.rdfile(root.url))
            editor.text = saveCache[saveCache.length - 1]
        }
    }

    function updateCache() {
        var index = -1
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i] === root.url) {
                index = i
                break
            }
        }
        if (index >= 0) saveCache[index] = editor.text
    }

    function getClearSaves() {
        var clearSaves = []
        var clearUrls = []
        var toolDir = folderBrowser.rootFolder.toString().replace("file://","")
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i].indexOf(toolDir) >= 0) {
                clearUrls.push(urlCache[i])
                clearSaves.push(FileManager.rdfile(urlCache[i]))
            }
        }
        return [clearUrls, clearSaves]
    }

    function flushCachesToDisk() {
        var toolDir = folderBrowser.rootFolder.toString().replace("file://","")
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i].indexOf(toolDir) >= 0) {
                var ret = FileManager.svfile(urlCache[i], saveCache[i])
            }
        }
    }

    function revertClearSavesToDisk(clearSvs) {
        var clearUrls = clearSvs[0]
        var clearSaves = clearSvs[1]
        for (var i = 0; i < clearUrls.length; i++) {
            FileManager.svfile(clearUrls[i], clearSaves[i])
        }
    }

    property string splitState: 'splitted'
    property int lineNumberPadding: Fit.fit(20)
    property int lineNumberSpacing: 0
    property var lastItem: null
    property bool toolboxMode: false

    property alias folder: folderBrowser.folder
    property alias rootFolder: folderBrowser.rootFolder
    property string url
    property var urlCache: []
    property var saveCache: []
    signal saved(string code)

    function show(url) {
        DelayCaller.delayCall(500, function() {
            var name = FileManager.fname(url)
            for (var i = 0; i < folderBrowser.count; i++ ) {
                if (folderBrowser.itemAt(i) === name) {
                    folderBrowser.currentIndex = i
                    break
                }
            }
        })
    }

    function randomFileName() {
        function s4() {
            return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
        }
        return s4() + s4();
    }

    function reloadView() {
        if (lastItem != null) lastItem.destroy()
        for (var i=0; i<lineNumberRepeater.count; i++) lineNumberRepeater.itemAt(i).bgcolor = 'transparent'
        errorMessage.text = ""

        if (!toolboxMode) {
            try {
                lastItem = Qt.createQmlObject(editor.text, view);
            } catch (e) {
                var error = e.qmlErrors[0];
                errorMessage.text = "Error, line "+error.lineNumber+":"+error.columnNumber+" : "+error.message;
                lineNumberRepeater.itemAt(error.lineNumber - 1).bgcolor = 'red'
                return;
            }
        } else {
            updateCache()
            var clearSaves = getClearSaves()
            flushCachesToDisk()
            var toolDir = folderBrowser.rootFolder.toString().replace("file://","")
            var mainFileName = toolDir + "/main.qml"

            lastItem = null
            var component = Qt.createComponent("file://" + mainFileName)
            if (component.status === Component.Ready) {
                lastItem = component.createObject(view);
                if (lastItem == null) {
                    errorMessage.text = "Error code 0x54."
                }
            } else {
                var regex = /file:\/\/.*(:)/g
                var festring = component.errorString().split("\n")
                var estring = festring[festring.length - 2]
                var fpath = estring.match(regex)[0].replace("file://","").slice(0, -1)
                var fname = FileManager.fname(fpath)
                var cleare = estring.replace(regex, "")
                var line = cleare.match(/[0-9]+/g)[0]
                var message = cleare.match(/[^0-9].*/g)[0]
                var ferror = "Error, line " + line + " in " + fname + ":" + message
                errorMessage.text = ferror
                if (FileManager.fname(root.url) === fname) {
                    lineNumberRepeater.itemAt(line - 1).bgcolor = 'red'
                }
            }

            revertClearSavesToDisk(clearSaves)
            cacheCleaner.clear()
            FileManager.rmsuffix(toolDir, "qmlc")
        }

        if (lastItem != null) {
            lastItem.width = Fit.fit(lastItem.width)
            lastItem.height = Fit.fit(lastItem.height)
            lastItem.anchors.centerIn = view
        }
    }

    function isTextFile(file) {
        return (FileManager.ftype(file) === "txt" ||
                FileManager.ftype(file) === "qml" ||
                FileManager.ftype(file) === "js")
    }
}
