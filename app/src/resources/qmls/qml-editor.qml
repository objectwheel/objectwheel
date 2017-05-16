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
//TODO: Fix android logo
//TODO: Add IOS/MacOS/Linux/Windows logo on each platform
//TODO: Fix objectwheel's to capital "Objectwheel" everywhere
//TODO: Fix warnings on Visual Studio
//TODO: Fix qml file names in resource
//TODO: Fix bugs on binding editor
//TODO: Add "Fit" lib to com.objectwheel.components
//FIX: Editor "error" line corruption when page word wrapped
//TODO: That alignment lock bar/layout bar
//FIX: Prompt user if it exit before save qml editor pages
//TODO: Copy-Cut-Paste operations, select,select-all operations on folders/files

Item {
    id: root
    onVisibleChanged: if (!visible) componentManager.clear()
    FileExplorer {
        id: fileExplorer
        clip: true
        readOnly: ["main.qml", "icon.png"]
        anchors { top: parent.top; bottom: parent.bottom; }
        width: Fit.fit(180)
        x: menu.checked ? 0 : -width
        Component.onCompleted: anim.enabled = true
        dropIcon: "qrc:///resources/images/fileExplorer/drop.png"
        explorerToolBar {
            backIcon: "qrc:///resources/images/fileExplorer/back.png"
            homeIcon: "qrc:///resources/images/fileExplorer/home.png"
            newFolderIcon: "qrc:///resources/images/fileExplorer/newFolder.png"
            newFileIcon: "qrc:///resources/images/fileExplorer/newFile.png"
            downloadIcon: "qrc:///resources/images/fileExplorer/download.png"
        }
        explorerListView {
            folderListModel.showDirsFirst: true
            folderListModel.hiddenSuffixes: ["qmlc", "jsc"]
            elementHeight: Fit.fit(40)
            textSize: Fit.fit(12)
            textWidth: Fit.fit(120)
            textColor: "white"
            imageIcon: "qrc:///resources/images/fileExplorer/image.svg"
            qmlIcon: "qrc:///resources/images/fileExplorer/qml.svg"
            jsIcon: "qrc:///resources/images/fileExplorer/js.svg"
            textIcon: "qrc:///resources/images/fileExplorer/text.png"
            binIcon: "qrc:///resources/images/fileExplorer/bin.svg"
            dirIcon: "qrc:///resources/images/fileExplorer/dir.png"
        }
        onEntryRenamed: {
            from = from.toString().replace("file://", "")
            to = to.toString().replace("file://", "")
            updateCacheForRenamedEntry(from, to, isdir)
        }
        onEntryDeleted: {
            name = name.toString().replace("file://", "")
            clearCacheFor(name, isdir)
        }
        explorerListView.listView.onCurrentItemChanged: {
            var urlval = explorerListView.folderListModel.get(explorerListView.listView.currentIndex, "filePath")
            var isdir = explorerListView.folderListModel.isFolder(explorerListView.listView.currentIndex)
            if (!isdir && isTextFile(urlval)) {
                setUrl(urlval)
                imageViewer.visible = false
                editor.visible = true
                currFileNameText.text = FileManager.fname(urlval)
            } else if (!isdir && isImageFile(urlval)) {
                imageViewer.source = componentManager.urlOfPath(urlval)
                imageViewer.visible = true
                editor.visible = false
                currFileNameText.text = FileManager.fname(urlval)
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
        anchors { left:fileExplorer.right; top:parent.top; bottom: parent.bottom; }
        width: parent.width
        clip: true
        color: "#4E5B67"
        Rectangle {
            id:toolBar
            z: 2
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: Fit.fit(42)
            color: "#0D74C8"
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
                    onClicked: {
                        saveCurrent();
                        if (!toolboxMode) root.saved(root.url.toLocaleString());
                    }
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
                    clip: true
                    anchors { top: parent.top; left: parent.left; bottom: separator.bottom; right:separator.left }
                    editor.font.pixelSize: fontSlider.value
                }
                ImageViewer {
                    id: imageViewer
                    anchors.fill: editor
                    visible: false
                    clip: true
                    defaultSize: Fit.fit(80)
                    warningImageSource: "qrc:///resources/images/sign_warning.png"
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
            color: "#6BB64B"
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
                            errorHead.width = Qt.binding(function(){return Math.floor(container.width / 1.618)})
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

            Rectangle {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                height: currFileNameText.contentHeight + Fit.fit(3)
                width: currFileNameText.contentWidth + Fit.fit(10)
                radius: Fit.fit(3)
                color: "#0D74C8"
                Rectangle {
                    anchors.right: parent.right
                    height: parent.height
                    width: parent.radius
                }

                Text {
                    id: currFileNameText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: Fit.fit(5)
                    text: qsTr("filename.qml")
                    font.pixelSize: Fit.fit(12)
                    color: "white"
                }
            }

            Slider {
                id: fontSlider
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: Fit.fit(40)
                updateValueWhileDragging: true
                tickmarksEnabled: true
                value: Fit.fit(12)
                maximumValue: Fit.fit(24)
                minimumValue: Fit.fit(6)
                stepSize: 3
                width: Fit.fit(100)
            }

        }
        Timer {
            id: timer
            interval: 500;
            onTriggered: reloadView()
            Component.onCompleted: editor.textChanged.connect(restart)
        }
    }
    ShadowFactory {
        targets: [errorHead, container, navibar, handleImage, toolBar]
        places: [Item.Bottom, Item.Left, Item.Bottom, Item.Bottom, Item.Bottom]
    }
    ComponentManager { id: componentManager }
    function setUrl(urlval) {
        root.url = urlval
        var index = -1
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i] === root.url) {
                index = i
                break
            }
        }
        if (index >= 0) {
            editor.editor.text = saveCache[index]
        } else {
            urlCache.push(root.url)
            saveCache.push(FileManager.rdfile(root.url))
            editor.editor.text = saveCache[saveCache.length - 1]
        }
        reloadView()
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
        setUrl(url)
        var name = FileManager.fname(url)
        var fm = fileExplorer.explorerListView.folderListModel
        for (var i = 0; i < fm.count; i++ ) {
            if (fm.get(i, "fileName") === name) {
                fileExplorer.explorerListView.listView.currentIndex = i
                break
            }
        }
    }
    function setRootFolder(path) {
        fileExplorer.explorerListView.folderListModel.rootFolder = ""
        fileExplorer.explorerListView.folderListModel.rootFolder = path
    }
    function setFolder(path) {
        fileExplorer.explorerListView.folderListModel.folder = ""
        fileExplorer.explorerListView.folderListModel.folder = path
    }
    function setToolboxMode(val) {
        toolboxMode = val
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
        var toolDir = fileExplorer.explorerListView.folderListModel.rootFolder.toString().replace("file://","")
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i].indexOf((toolDir + FileManager.separator())) >= 0) {
                clearUrls.push(urlCache[i])
                clearSaves.push(FileManager.rdfile(urlCache[i]))
            }
        }
        return [clearUrls, clearSaves]
    }
    function flushCachesToDisk() {
        var toolDir = fileExplorer.explorerListView.folderListModel.rootFolder.toString().replace("file://","")
        for (var i = 0; i < urlCache.length; i++) {
            if (urlCache[i].indexOf((toolDir + FileManager.separator())) >= 0) {
                var ret = FileManager.wrfile(urlCache[i], saveCache[i])
            }
        }
    }
    function revertClearSavesToDisk(clearSvs) {
        var clearUrls = clearSvs[0]
        var clearSaves = clearSvs[1]
        for (var i = 0; i < clearUrls.length; i++) {
            FileManager.wrfile(clearUrls[i], clearSaves[i])
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

            for (var j=0; j<fileExplorer.explorerListView.folderListModel.count;j++) {
                if (fileExplorer.explorerListView.folderListModel.get(j,"fileName") === "main.qml") {
                    fileExplorer.explorerListView.listView.currentIndex = j
                }
            }
        } else if (isTextFile(urlval)) {
            for (var ii = 0; ii < urlCache.length; ii++) {
                if (urlCache[ii] === urlval) {
                    urlCache.splice(ii, 1)
                    saveCache.splice(ii, 1)
                }
            }

            for (var jj=0; jj<fileExplorer.explorerListView.folderListModel.count;jj++) {
                if (fileExplorer.explorerListView.folderListModel.get(jj,"fileName") === "main.qml") {
                    fileExplorer.explorerListView.listView.currentIndex = jj
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
                    FileManager.wrfile(to, saveCache[ii])
                    urlCache.splice(ii, 1)
                    saveCache.splice(ii, 1)

                    for (var j=0; j<fileExplorer.explorerListView.folderListModel.count;j++) {
                        if (fileExplorer.explorerListView.folderListModel.get(j,"fileName") === "main.qml") {
                            fileExplorer.explorerListView.listView.currentIndex = j
                        }
                    }
                }
            }
        } else if (!isTextFile(from) && isTextFile(to)) {
            setUrl(to)
        }

        reloadView()
    }
    function reloadView() {
        if (lastItem != null) lastItem.destroy()
        if (!root.visible) return
        for (var i=0; i<editor.lineNumberRepeater.count; i++) editor.lineNumberRepeater.itemAt(i).bgcolor = 'transparent'
        errorMessage.text = ""
        componentManager.clear()

        updateCache()
        var clearSaves = getClearSaves()
        flushCachesToDisk()

        if (!componentManager.build(fileExplorer.explorerListView.folderListModel.rootFolder.toString().replace("file://", "") + "/main.qml")) {
            var errObj = JSON.parse(componentManager.errors()[0])
            if (root.url === errObj.path) {
                editor.lineNumberRepeater.itemAt(errObj.line - 1).bgcolor = "#c74c3c"
            }
            errorMessage.text = "<b>Error, line " + errObj.line + ':' + errObj.column + " in " +  FileManager.fname(errObj.path) + " : </b>" + errObj.description
        }

        revertClearSavesToDisk(clearSaves)

    }
    function isTextFile(file) {
        return (FileManager.ftype(file) === "txt" ||
                FileManager.ftype(file) === "qml" ||
                FileManager.ftype(file) === "js")
    }
    function isImageFile(file) {
        return (FileManager.ftype(file) === "img")
    }
    property alias view: view
    property real maxWidth: 0
    property string splitState: 'splitted'
    property var lastItem: null
    property bool toolboxMode: false
    property string folder
    property string rootFolder
    property string url
    property var urlCache: []
    property var saveCache: []
    signal saved(string qmlPath)
    signal currentSaved()
}
