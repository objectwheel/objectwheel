import QtQuick 2.7
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
//TODO: Show parent folder in folderList's upside
//FIX: Main.qml should not be removed and icon
//FIX: Editor "error" line corruption when page word wrapped
//FIX: Clear "jsx" and "qmlc" cache files recursively
//TODO: Add image viewer
//TODO: Fix binding loops

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
            updateCacheForRenamedEntry(from, to, isdir)
        }
        onEntryRemoved: {
            urlval = urlval.toString().replace("file://", "")
            clearCacheFor(urlval, isdir)
        }

        onSelectionChanged: {
            urlval = urlval.toString().replace("file://", "")
            if (!isdir && isTextFile(urlval)) {
                root.url = urlval
                imageViewer.visible = false
                flickable.visible = true
            } else if (!isdir && isImageFile(urlval)) {
                imageViewer.source = "file://" + urlval
                imageViewer.visible = true
                flickable.visible = false
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

    Rectangle {
        id: container
        anchors { left:folderBrowser.right; top:parent.top; bottom: parent.bottom; right: parent.right }
        clip: true
        color: "#44504e"
        Rectangle {
            id:toolBar
            z: 2
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: Fit.fit(42)
            color: "#2b5796"
            clip: true

            Timer {
                interval: 10000
                running: true
                onTriggered: separator.x += 80
            }

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
                    onCheckedChanged: {
                        if (checked) {
                            sepPrevXDiff = (separator.x - editorContainer.width/2.0)/(editorContainer.width)
                            fixerAnim.start()
                            fixerTimer.start()
                        } else {
                            fixerAnim.start()
                            fixerTimer.start()
                        }
                    }

                    NumberAnimation { id: fixerAnim; duration: 400; target:QtObject{} }

                    Timer {
                        id: fixerTimer
                        interval: 1
                        repeat: fixerAnim.running
                        onTriggered: {
                            separator.x = editorContainer.width/2.0 + menu.sepPrevXDiff * editorContainer.width
                        }
                    }

                    property real sepPrevXDiff
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
        Item {
            id: editorPadder
            anchors{
                left:parent.left
                right:parent.right
                top:toolBar.bottom
                bottom: navibar.top
            }
            Item {
                id: topPadder
                anchors { left:parent.left; right:parent.right; top:parent.top }
                height: Fit.fit(10)
            }
            Item {
                id: leftPadder
                anchors { left:parent.left; bottom:parent.bottom; top:parent.top }
                width: Fit.fit(10)
            }
            Item {
                id: rightPadder
                anchors { right:parent.right; bottom:parent.bottom; top:parent.top }
                width: Fit.fit(10)
            }
            Item  {
                id: editorContainer
                anchors{
                    left: leftPadder.right
                    right: rightPadder.left
                    top: topPadder.bottom
                    bottom: parent.bottom
                }

                Item {
                    id: view
                    state: root.splitState
                    anchors { top: errorHead.bottom; right: parent.right; bottom: separator.bottom; left: separator.right }
                    enabled: opacity > 0.98 ? true : false
                    clip: true
                    states: [
                        State {
                            name: "splitted"
                            PropertyChanges { target: view; opacity: 1 }
                            PropertyChanges { target: flickable; opacity: 1 }
                            PropertyChanges { target: flickable; opacity: 1 }
                            PropertyChanges { target: handle; opacity: 1 }
                            PropertyChanges { target: separator; x:parent.width/2.0 }
                        },
                        State {
                            name: "editor"
                            PropertyChanges { target: view; opacity: 0}
                            PropertyChanges { target: flickable; opacity: 1 }
                            PropertyChanges { target: imageViewer; opacity: 1}
                            PropertyChanges { target: handle; opacity: 0 }
                            PropertyChanges { target: separator; x: parent.width - Fit.fit(1)}
                        },
                        State {
                            name: "viewer"
                            PropertyChanges { target: view; opacity: 1 }
                            PropertyChanges { target: flickable; opacity: 0}
                            PropertyChanges { target: imageViewer; opacity: 0}
                            PropertyChanges { target: handle; opacity: 0 }
                            PropertyChanges { target: separator; x: 0 }
                        }
                    ]
                    transitions: [
                        Transition {
                            to: "*"
                            NumberAnimation { target: separator; properties: "x"; duration: 300; easing.type: Easing.InOutQuad; }
                            NumberAnimation { target: view; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                            NumberAnimation { target: flickable; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                            NumberAnimation { target: imageViewer; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                            NumberAnimation { target: handle; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                        }
                    ]
                }
                Flickable {
                    id: flickable
                    anchors { top: parent.top; left: parent.left; bottom: separator.bottom; right:separator.left }
                    enabled: opacity > 0.98 ? true : false
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.DragOverBounds
                    contentHeight: editor.paintedHeight
                    clip: true
                    ScrollBar.vertical: ScrollBar { }

                    Column {
                        id: lineNumber
                        width: maxWidth
                        anchors { left: parent.left; top: parent.top }
                        Repeater {
                            id: lineNumberRepeater
                            model: editor.lineCount
                            clip: true
                            Text {
                                property alias bgcolor: rect.color
                                text: index + 1
                                color: "#e0e0e0"
                                width: maxWidth
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
                        anchors.right: lineNumber.right
                        anchors.rightMargin: lineNumber.width - maxWidth
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
                        anchors { left: lineNumber.right; right: parent.right; top: parent.top; bottom: parent.bottom }
                        wrapMode: flickable.opacity > 0.98 ? TextEdit.WrapAtWordBoundaryOrAnywhere : TextEdit.NoWrap ;
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
                            visible: !isDesktop
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
                ImageViewer {
                    id: imageViewer
                    anchors.fill: flickable
                    visible: false
                    clip: true
                    defaultSize: Fit.fit(80)
                }
                Rectangle {
                    id: separator
                    x: parent.width/2.0
                    color: Qt.lighter(container.color, 1.5)
                    width: Fit.fit(1)
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    onXChanged: {
                        if (view.state !== "splitted") {
                            if (x === (editorContainer.width - Fit.fit(1)) || x === 0) {
                                visible = false
                            } else {
                                visible = true
                            }
                        } else {
                            visible = true
                        }
                    }

                    MouseArea {
                        id: handle
                        height: Fit.fit(22)
                        width: height
                        anchors.centerIn: parent
                        drag.target: separator;
                        drag.axis: "XAxis"
                        drag.minimumX: 0
                        drag.maximumX: editorContainer.width - Fit.fit(1)
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
            }
        }
        NaviBar {
            id: navibar
            height: Fit.fit(49)
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            state: "view"
        }
        Timer {
            id: timer
            interval: 500;
            onTriggered: reloadView()
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
            if (urlCache[i].indexOf((toolDir + FileManager.separator())) >= 0) {
                clearUrls.push(urlCache[i])
                clearSaves.push(FileManager.rdfile(urlCache[i]))
            }
        }
        return [clearUrls, clearSaves]
    }

    function flushCachesToDisk() {
        var toolDir = folderBrowser.rootFolder.toString().replace("file://","")
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i].indexOf((toolDir + FileManager.separator())) >= 0) {
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

    function clearCache() {
        urlCache = []
        saveCache = []
    }

    function clearCacheFor(urlval, isdir) {
        if (isdir) {
            for (var i = 0; i < urlCache.length; i++) {
                if (urlCache[i].indexOf(urlval + FileManager.separator()) >= 0) {
                    urlCache.splice(i, 1)
                    saveCache.splice(i, 1)
                }
            }

            for (var j=0; j<folderBrowser.count;j++) {
                if (folderBrowser.itemAt(j) === "main.qml") {
                    folderBrowser.currentIndex = j
                }
            }
        } else if (isTextFile(urlval)) {
            for (var ii = 0; ii < urlCache.length; ii++) {
                if (urlCache[ii] === urlval) {
                    urlCache.splice(ii, 1)
                    saveCache.splice(ii, 1)
                }
            }

            for (var jj=0; jj<folderBrowser.count;jj++) {
                if (folderBrowser.itemAt(jj) === "main.qml") {
                    folderBrowser.currentIndex = jj
                }
            }
        }
    }

    function updateCacheForRenamedEntry(from, to, isdir) {
        if (isdir) {
            var updateSubCaches = function(dirold, dirnew) {
                var fileList = FileManager.lsfile(dirnew)
                for (var i = 0; i < fileList.length; i++) {
                    for (var ii = 0; ii < urlCache.length; ii++) {
                        if (urlCache[ii] === (dirold + FileManager.separator() + fileList[i])) {
                            urlCache[ii] = (dirnew + FileManager.separator() + fileList[i])
                        }
                    }
                }

                var dirList = FileManager.lsdir(dirnew)
                for (var j = 0; j < dirList.length; j++) {
                    updateSubCaches(dirold + FileManager.separator() + dirList[j], dirnew + FileManager.separator() + dirList[j])
                }
            }
            updateSubCaches(from, to)
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
                lineNumberRepeater.itemAt(error.lineNumber - 1).bgcolor = "#c74c3c"
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
                var regex = /file:\/\/.*?:/g
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
                    lineNumberRepeater.itemAt(line - 1).bgcolor = "#c74c3c"
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

    function isImageFile(file) {
        return (FileManager.ftype(file) === "img")
    }

    property real maxWidth: 0
    property string splitState: 'splitted'
    property var lastItem: null
    property bool toolboxMode: false
    property alias folder: folderBrowser.folder
    property alias rootFolder: folderBrowser.rootFolder
    property string url
    property var urlCache: []
    property var saveCache: []
    signal saved(string code)
}
