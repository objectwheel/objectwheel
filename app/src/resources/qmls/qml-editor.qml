import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import com.objectwheel.components 1.0
import com.objectwheel.editor 1.0
import "fit.js" as Fit
import "filemanager.js" as FileManager
import "delaycaller.js" as DelayCaller

//TODO: Fix keyboard problems of qml editor
//TODO: Fix popup window closing method
//TODO: Fix android logo
//TODO: Add IOS/MacOS/Linux/Windows logo on each platform
//TODO: Fix objectwheel's to capital "Objectwheel" everywhere
//TODO: Fix warnings on Visual Studio
//TODO: Fix qml file names in resource
//TODO: Fix bugs on binding editor
//TODO: Fix popup window colors
//TODO: Add "Fit" lib to com.objectwheel.components
//FIX: Editor "error" line corruption when page word wrapped
//FIX: Unexpectedly finishing problem because of CacheCleaner
//FIX: Jumping image viewer
//TODO: That alignment lock bar/layout bar
//TODO: Add animation popup to toolbox adder area

Item {
    id: root

    FolderBrowser {
        id: folderBrowser
        clip: true
        readOnly: ["main.qml", "icon.png"]
        anchors { top: parent.top; bottom: parent.bottom; }
        width: Fit.fit(180)
        x: menu.checked ? 0 : -width
        enabled: toolboxMode
        hiddenExtensions: [ "qmlc", "jsc" ]
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
                editor.visible = true
            } else if (!isdir && isImageFile(urlval)) {
                imageViewer.source = "file://" + urlval
                imageViewer.visible = true
                editor.visible = false
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
        anchors { left:folderBrowser.right; top:parent.top; bottom: parent.bottom; }
        width: parent.width
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

            RowLayout {
                anchors.margins: Fit.fit(6)
                anchors.fill: parent
                spacing: Fit.fit(3)

                FancyButton {
                    id: menu
                    width: height
                    checkable: true
                    height: parent.height
                    iconSource: "qrc:///resources/images/fexplorer.png"
                }

                Item { Layout.fillWidth: true }

                FancyButton {
                    id: save
                    width: height
                    height: parent.height
                    iconSource: "qrc:///resources/images/save-icon.png"
                    onClicked: !toolboxMode ? root.saved(editor.editor.text) : saveCurrent()
                }

                FancyButton {
                    id: undo
                    width: height
                    height: parent.height
                    enabled: editor.editor.canUndo
                    iconSource: "qrc:///resources/images/left-arrow.png"
                    onClicked: editor.editor.undo()
                }

                FancyButton {
                    id: redo
                    width: height
                    height: parent.height
                    enabled: editor.editor.canRedo
                    iconSource: "qrc:///resources/images/right-arrow.png"
                    onClicked: editor.editor.redo()
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
                    anchors { top: parent.top; right: parent.right; bottom: separator.bottom; left: separator.right }
                    enabled: opacity > 0.98 ? true : false
                    clip: true
                    states: [
                        State {
                            name: "splitted"
                            PropertyChanges { target: view; opacity: 1 }
                            PropertyChanges { target: editor; opacity: 1 }
                            PropertyChanges { target: editor; opacity: 1 }
                            PropertyChanges { target: handle; opacity: 1 }
                            PropertyChanges { target: separator; x:parent.width/2.0 }
                        },
                        State {
                            name: "editor"
                            PropertyChanges { target: view; opacity: 0}
                            PropertyChanges { target: editor; opacity: 1 }
                            PropertyChanges { target: imageViewer; opacity: 1}
                            PropertyChanges { target: handle; opacity: 0 }
                            PropertyChanges { target: separator; x: parent.width - Fit.fit(1)}
                        },
                        State {
                            name: "viewer"
                            PropertyChanges { target: view; opacity: 1 }
                            PropertyChanges { target: editor; opacity: 0}
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
                            NumberAnimation { target: editor; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                            NumberAnimation { target: imageViewer; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                            NumberAnimation { target: handle; properties: "opacity"; duration: 300; easing.type: Easing.InOutQuad; }
                        }
                    ]
                }
                TextEditor {
                    id: editor
                    navbar: navibar
                    timer: timer
                    anchors { top: parent.top; left: parent.left; bottom: separator.bottom; right:separator.left }
                }
                ImageViewer {
                    id: imageViewer
                    anchors.fill: editor
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
               }
        }
        Rectangle {
            id: errorHead
            clip: true
            color: "#1e8145"
            radius: Fit.fit(5)
            x:  Math.floor(container.width/2.0 - errorHead.width/2.0)
            width: Math.floor(parent.width / 1.618)
            height: Fit.fit(40) + radius
            y: Math.floor(-height + toolBar.height - radius)

            Behavior on y {
                NumberAnimation { duration: 500; easing.type: Easing.OutExpo }
            }

            Behavior on width {
                NumberAnimation { duration: 500; easing.type: Easing.OutExpo }
            }

            Text {
                id: errorMessage
                anchors{top:parent.top;left:parent.left;right:parent.right;bottom:parent.bottom;margins:Fit.fit(5)}
                wrapMode: Text.WordWrap
                color: "white"
                clip: true
                font.pixelSize: Fit.fit(11)
                renderType: Text.NativeRendering
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Behavior on opacity {
                    NumberAnimation { duration: 500; easing.type: Easing.OutExpo }
                }
            }

            Component.onCompleted: {
                errorMessage.textChanged.connect(handleError)
            }

            function handleError() {
                if (errorMessage.text != "") {
                    y = Math.floor(toolBar.height  - radius)
                } else {
                    y = - Math.floor(height + toolBar.height - radius)
                }
            }

            Text {
                anchors { right: parent.right; bottom: parent.bottom; margins:Fit.fit(5) }
                text: "➤"
                width: Fit.fit(15)
                height: Fit.fit(15)
                color: "white"
                font.bold: true
                clip: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter

                FancyButton {
                    anchors.fill: parent
                    cursorMark: Qt.PointingHandCursor
                    checkable: true
                    onCheckedChanged: {
                        if (checked) {
                            parent.rotation = 180
                            errorMessage.opacity = 0.0
                            parent.width = Fit.fit(15)
                            parent.height = Fit.fit(15)
                            parent.anchors.right = undefined
                            parent.anchors.left = errorHead.left
                            errorHead.anchors.horizontalCenter = undefined
                            errorHead.anchors.right = container.right
                            errorHead.width = Fit.fit(30)
                        } else {
                            parent.rotation = 0
                            errorMessage.opacity = 1.0
                            parent.width = Fit.fit(15)
                            parent.height = Fit.fit(15)
                            parent.anchors.right = errorHead.right
                            parent.anchors.left = undefined
                            errorHead.anchors.horizontalCenter = container.horizontalCenter
                            errorHead.anchors.right = undefined
                            errorHead.width = Math.floor(container.width / 1.618)
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
        targets: [errorHead, container, navibar, handleImage, toolBar]
        places: [Item.Bottom, Item.Left, Item.Bottom, Item.Bottom, Item.Bottom]
    }

    CacheCleaner { id: cacheCleaner }

    onToolboxModeChanged: {
        if (!toolboxMode) {
            imageViewer.visible = false
            editor.visible = true
        }
    }

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
            editor.editor.text = saveCache[index]
        } else {
            urlCache.push(root.url)
            saveCache.push(FileManager.rdfile(root.url))
            editor.editor.text = saveCache[saveCache.length - 1]
        }
    }

    function saveCurrent() {
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i].indexOf(FileManager.dname(url) + FileManager.separator()) >= 0) {
                FileManager.wrfile(urlCache[i], saveCache[i])
            }
        }
        toolboxMode = false
        currentSaved()
    }

    function show(url) {
        var name = FileManager.fname(url)
        var entries = FileManager.ls(FileManager.dname(url))
        for (var i = 0; i < entries.length; i++ ) {
            if (entries[i] === name) {
                folderBrowser.currentIndex = i
                DelayCaller.delayCall(500, function() {
                    folderBrowser.currentIndex = i
                })
                break
            }
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
        if (index >= 0) saveCache[index] = editor.editor.text
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

    function reloadView() {
        if (lastItem != null) lastItem.destroy()
        for (var i=0; i<editor.lineNumberRepeater.count; i++) editor.lineNumberRepeater.itemAt(i).bgcolor = 'transparent'
        errorMessage.text = ""

        if (!toolboxMode) {
            try {
                lastItem = Qt.createQmlObject(editor.editor.text, view);
            } catch (e) {
                var error = e.qmlErrors[0];
                errorMessage.text = "Error, line "+error.lineNumber+":"+error.columnNumber+" : "+error.message;
                editor.lineNumberRepeater.itemAt(error.lineNumber - 1).bgcolor = "#c74c3c"
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
                    editor.lineNumberRepeater.itemAt(line - 1).bgcolor = "#c74c3c"
                }
            }

            revertClearSavesToDisk(clearSaves)
            cacheCleaner.clear()
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
    signal currentSaved()
}
