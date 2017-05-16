import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "fit.js" as Fit
import "filemanager.js" as FileManager

Rectangle {
    id: root
    color: "#434B52"
    FileExplorerToolBar {
        id: explorerToolBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: Fit.fit(42)
        color: "#6BB64B"
        onDownloadButtonClicked: { popup.containerItem = d.containers[0]; popup.open() }
        onNewFileButtonClicked: { popup.containerItem = d.containers[1]; popup.open() }
        onNewFolderButtonClicked: { popup.containerItem = d.containers[2]; popup.open() }
        onHomeButtonClicked: { explorerListView.goRoot() }
        onBackButtonClicked: { explorerListView.goBack() }
    }
    FileExplorerListView {
        id: explorerListView
        anchors {
            left: parent.left;  right: parent.right
            bottom: parent.bottom; top: explorerToolBar.bottom; margins: Fit.fit(2)
        }
        onItemPressedAndHeld: {
            var fname = explorerListView.folderListModel.get(index, "fileName")
            for (var i = 0; i < readOnly.length; i++) {
                if (readOnly[i].toLowerCase() === fname.toLowerCase()) {
                    return
                }
            }
            popup.containerItem = d.containers[3];
            popup.open()
        }
        onItemClicked: {
            if (mouse.button === Qt.RightButton) {
                var fname = explorerListView.folderListModel.get(index, "fileName")
                for (var i = 0; i < readOnly.length; i++) {
                    if (readOnly[i].toLowerCase() === fname.toLowerCase()) {
                        return
                    }
                }
                popup.containerItem = d.containers[3];
                popup.open()
            }
        }
        listView.onCurrentItemChanged: {
            popup.close()
        }
    }
    DropArea {
        anchors.fill: explorerListView
        enabled: !popup.opened
        Rectangle {
            anchors.fill: parent
            color: "#65000000"
            visible: parent.containsDrag
            Image {
                anchors.centerIn: parent
                source: dropIcon
                height: Fit.fit(69)
                width: Fit.fit(160)
                antialiasing: true
            }
        }
        onDropped: {
            if (drop.hasUrls) {
                for (var i = 0; i < drop.urls.length; i++) {
                    var from = drop.urls[i].toString().replace("file://","")
                    var to = explorerListView.folderListModel.folder.toString().replace("file://","")
                    console.log(from, to)
                    FileManager.cp(from, to)
                }
            }
        }
    }
    ShadowFactory {
        targets: [explorerToolBar]
        places: [Item.Bottom]
    }
    Popup {
        id: popup
        anchors.fill: parent
        color: "#70000000"
        containerItem: d.containers[0]
        containerColor: d.containerColor
        cancelButton.base.color: d.containerColor
        cancelButton.text.color: d.pressedButtonColor
        cancelButton.onPressed: {
            cancelButton.base.color = d.pressedButtonColor
            cancelButton.text.color = d.containerColor
        }
        cancelButton.onReleased: {
            cancelButton.base.color = d.containerColor
            cancelButton.text.color = d.pressedButtonColor
        }
        z:3
    }
    QtObject {
        id: d
        property list<Item> containers: [
            Item {
                id: downloadContainer
                height: Fit.fit(85)
                visible: false
                TextField {
                    id: textField
                    anchors.top: parent.top
                    anchors.topMargin: Fit.fit(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - Fit.fit(20)
                    implicitHeight: Fit.fit(30)
                    placeholderText: "Enter url"
                    horizontalAlignment: Text.AlignHCenter
                    onVisibleChanged: { text = ""; focus = false }
                    Keys.onReturnPressed: {
                        if (!topRect.enabled) return
                        topRect.color = d.pressedButtonColor
                        bottomRect.color = d.pressedButtonColor
                        bottomItem.clr = d.containerColor
                        var dir = root.explorerListView.folderListModel.folder.toString().replace("file://","")
                        var el = FileManager.ls(dir)
                        var name = FileManager.fname(textField.text)
                        for (var j = 0; j < el.length; j++) {
                            if (name.toLowerCase() === el[j].toLowerCase()) {
                                name = "copy_" + name
                                j = -1
                            }
                        }
                        var data = FileManager.dlfile(textField.text)
                        var lf = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/" + name
                        FileManager.wrfile(lf, data)
                        topRect.color = d.containerColor
                        bottomRect.color = d.containerColor
                        bottomItem.clr = d.textColor
                        popup.close()
                    }
                    style: TextFieldStyle {
                        textColor: d.textColor
                        selectionColor: d.containerColor
                        placeholderTextColor: Qt.darker(d.textColor, 1.2)
                        background: Rectangle {
                            border.color: d.lineColor
                            border.width: Fit.fit(1)
                            radius: Fit.fit(5)
                            color: "#20ffffff"
                        }
                    }
                }
                Rectangle {
                    id: line
                    anchors.top: textField.bottom
                    anchors.topMargin: Fit.fit(10)
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: Fit.fit(1)
                    color: d.lineColor
                }
                Item {
                    id: bottomItem
                    anchors.top: line.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    enabled: textField.text !== ""
                    Rectangle {
                        id: topRect
                        color: d.containerColor
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: Fit.fit(10)
                    }
                    Rectangle {
                        id: bottomRect
                        color: d.containerColor
                        anchors.fill: parent
                        radius: Fit.fit(8)
                    }
                    Text {
                        id: label
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "Download"
                        color: enabled ? parent.clr : Qt.darker(d.textColor, 1.2)
                    }
                    property color clr: d.textColor
                    MouseArea {
                        id: msa
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onPressed: {
                            topRect.color = d.pressedButtonColor
                            bottomRect.color = d.pressedButtonColor
                            parent.clr = d.containerColor
                        }
                        onReleased: {
                            topRect.color = d.containerColor
                            bottomRect.color = d.containerColor
                            parent.clr = d.textColor
                        }
                        onClicked: {
                            topRect.color = d.pressedButtonColor
                            bottomRect.color = d.pressedButtonColor
                            parent.clr = d.containerColor
                            var dir = root.explorerListView.folderListModel.folder.toString().replace("file://","")
                            var el = FileManager.ls(dir)
                            var name = FileManager.fname(textField.text)
                            for (var j = 0; j < el.length; j++) {
                                if (name.toLowerCase() === el[j].toLowerCase()) {
                                    name = "copy_" + name
                                    j = -1
                                }
                            }
                            var data = FileManager.dlfile(textField.text)
                            var lf = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/" + name
                            FileManager.wrfile(lf, data)
                            topRect.color = d.containerColor
                            bottomRect.color = d.containerColor
                            parent.clr = d.textColor
                            popup.close()
                        }
                    }
                }
            },
            Item {
                id: newFileContainer
                height: Fit.fit(85)
                visible: false
                TextField {
                    id: textField2
                    anchors.top: parent.top
                    anchors.topMargin: Fit.fit(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - Fit.fit(20)
                    implicitHeight: Fit.fit(30)
                    placeholderText: "Enter file name"
                    horizontalAlignment: Text.AlignHCenter
                    onVisibleChanged: { text = ""; focus = false }
                    Keys.onReturnPressed: {
                        if (!topRect2.enabled) return
                        var name = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/" + textField2.text
                        FileManager.mkfile(name)
                        popup.close()
                    }
                    style: TextFieldStyle {
                        textColor: d.textColor
                        selectionColor: d.containerColor
                        placeholderTextColor: Qt.darker(d.textColor, 1.2)
                        background: Rectangle {
                            border.color: d.lineColor
                            border.width: Fit.fit(1)
                            radius: Fit.fit(5)
                            color: "#20ffffff"
                        }
                    }
                }
                Rectangle {
                    id: line2
                    anchors.top: textField2.bottom
                    anchors.topMargin: Fit.fit(10)
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: Fit.fit(1)
                    color: d.lineColor
                }
                Item {
                    anchors.top: line2.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    enabled: {
                        if (textField2.text === "") {
                            return false
                        } else {
                            var dir = root.explorerListView.folderListModel.folder.toString().replace("file://","")
                            var el = FileManager.ls(dir)
                            for (var j = 0; j < el.length; j++) {
                                if (textField2.text.toLowerCase() === el[j].toLowerCase()) {
                                    return false
                                }
                            }
                            return true
                        }
                    }
                    Rectangle {
                        id: topRect2
                        color: d.containerColor
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: Fit.fit(10)
                    }
                    Rectangle {
                        id: bottomRect2
                        color: d.containerColor
                        anchors.fill: parent
                        radius: Fit.fit(8)
                    }
                    Text {
                        id: label2
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "Create file"
                        color: enabled ? (msa2.pressed ? d.containerColor : d.textColor) : Qt.darker(d.textColor, 1.2)
                    }

                    MouseArea {
                        id: msa2
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onPressed: {
                            topRect2.color = d.pressedButtonColor
                            bottomRect2.color = d.pressedButtonColor
                        }
                        onReleased: {
                            topRect2.color = d.containerColor
                            bottomRect2.color = d.containerColor
                        }
                        onClicked: {
                            var name = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/" + textField2.text
                            FileManager.mkfile(name)
                            popup.close()
                        }
                    }
                }
            },
            Item {
                id: newFolderContainer
                height: Fit.fit(85)
                visible: false
                TextField {
                    id: textField3
                    anchors.top: parent.top
                    anchors.topMargin: Fit.fit(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - Fit.fit(20)
                    implicitHeight: Fit.fit(30)
                    placeholderText: "Enter folder name"
                    horizontalAlignment: Text.AlignHCenter
                    onVisibleChanged: { text = ""; focus = false }
                    Keys.onReturnPressed: {
                        if (!topRect3.enabled) return
                        var name = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/" + textField3.text
                        FileManager.mkdir(name)
                        popup.close()
                    }
                    style: TextFieldStyle {
                        textColor: d.textColor
                        selectionColor: d.containerColor
                        placeholderTextColor: Qt.darker(d.textColor, 1.2)
                        background: Rectangle {
                            border.color: d.lineColor
                            border.width: Fit.fit(1)
                            radius: Fit.fit(5)
                            color: "#20ffffff"
                        }
                    }
                }
                Rectangle {
                    id: line3
                    anchors.top: textField3.bottom
                    anchors.topMargin: Fit.fit(10)
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: Fit.fit(1)
                    color: d.lineColor
                }
                Item {
                    anchors.top: line3.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    enabled: {
                        if (textField3.text === "") {
                            return false
                        } else {
                            var dir = root.explorerListView.folderListModel.folder.toString().replace("file://","")
                            var el = FileManager.ls(dir)
                            for (var j = 0; j < el.length; j++) {
                                if (textField3.text.toLowerCase() === el[j].toLowerCase()) {
                                    return false
                                }
                            }
                            return true
                        }
                    }
                    Rectangle {
                        id: topRect3
                        color: d.containerColor
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: Fit.fit(10)
                    }
                    Rectangle {
                        id: bottomRect3
                        color: d.containerColor
                        anchors.fill: parent
                        radius: Fit.fit(8)
                    }
                    Text {
                        id: label3
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "Create folder"
                        color: enabled ? (msa3.pressed ? d.containerColor : d.textColor) : Qt.darker(d.textColor, 1.2)
                    }

                    MouseArea {
                        id: msa3
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onPressed: {
                            topRect3.color = d.pressedButtonColor
                            bottomRect3.color = d.pressedButtonColor
                        }
                        onReleased: {
                            topRect3.color = d.containerColor
                            bottomRect3.color = d.containerColor
                        }
                        onClicked: {
                            var name = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/" + textField3.text
                            FileManager.mkdir(name)
                            popup.close()
                        }
                    }
                }
            },
            Item {
                id: editEntryContainer
                height: Fit.fit(120)
                visible: false
                TextField {
                    id: textField4
                    anchors.top: parent.top
                    anchors.topMargin: Fit.fit(10)
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - Fit.fit(20)
                    implicitHeight: Fit.fit(30)
                    placeholderText: "Enter new name"
                    horizontalAlignment: Text.AlignHCenter
                    onVisibleChanged: {
                        var name = explorerListView.folderListModel.get(explorerListView.listView.currentIndex, "fileName")
                        if (typeof name !== "undefined") {
                            text = name
                            focus = true
                            selectAll()
                        }
                    }
                    Keys.onReturnPressed: {
                        if (!ttopRect.enabled) return
                        var folder = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/"
                        var oldName = folder + explorerListView.folderListModel.get(explorerListView.listView.currentIndex, "fileName")
                        var newName = folder + textField4.text
                        var isfolder = explorerListView.folderListModel.isFolder(explorerListView.listView.currentIndex)
                        FileManager.rn(oldName, newName)
                        popup.close()
                        entryRenamed("file://" + oldName, "file://" + newName, isfolder)
                    }
                    style: TextFieldStyle {
                        textColor: d.textColor
                        selectionColor: d.containerColor
                        placeholderTextColor: Qt.darker(d.textColor, 1.2)
                        background: Rectangle {
                            border.color: d.lineColor
                            border.width: Fit.fit(1)
                            radius: Fit.fit(5)
                            color: "#20ffffff"
                        }
                    }
                }
                Rectangle {
                    id: line4
                    anchors.top: textField4.bottom
                    anchors.topMargin: Fit.fit(10)
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: Fit.fit(1)
                    color: d.lineColor
                }
                Rectangle {
                    id: ttopRect
                    color: d.containerColor
                    anchors.top: line4.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: Fit.fit(34)
                    enabled: {
                        if (textField4.text === "") {
                            return false
                        } else {
                            var dir = root.explorerListView.folderListModel.folder.toString().replace("file://","")
                            var el = FileManager.ls(dir)
                            for (var j = 0; j < el.length; j++) {
                                if (textField4.text.toLowerCase() === el[j].toLowerCase()) {
                                    return false
                                }
                            }
                            return true
                        }
                    }
                    Text {
                        id: label4
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "Rename"
                        color: enabled ? (msa4.pressed ? d.containerColor : d.textColor) : Qt.darker(d.textColor, 1.2)
                    }
                    MouseArea {
                        id: msa4
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onPressed: {
                            ttopRect.color = d.pressedButtonColor
                        }
                        onReleased: {
                            ttopRect.color = d.containerColor
                        }
                        onClicked: {
                            var folder = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/"
                            var oldName = folder + explorerListView.folderListModel.get(explorerListView.listView.currentIndex, "fileName")
                            var newName = folder + textField4.text
                            var isfolder = explorerListView.folderListModel.isFolder(explorerListView.listView.currentIndex)
                            FileManager.rn(oldName, newName)
                            popup.close()
                            entryRenamed("file://" + oldName, "file://" + newName, isfolder)
                        }
                    }
                }
                Rectangle {
                    id: line5
                    anchors.top: ttopRect.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: Fit.fit(1)
                    color: d.lineColor
                }
                Item {
                    anchors.top: line5.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    Rectangle {
                        id: topRect4
                        color: d.containerColor
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: Fit.fit(10)
                    }
                    Rectangle {
                        id: bottomRect4
                        color: d.containerColor
                        anchors.fill: parent
                        radius: Fit.fit(8)
                    }
                    Text {
                        id: label5
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: "Delete"
                        color: d.deleteTextColor
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onPressed: {
                            topRect4.color = d.pressedButtonColor
                            bottomRect4.color = d.pressedButtonColor
                        }
                        onReleased: {
                            topRect4.color = d.containerColor
                            bottomRect4.color = d.containerColor
                        }
                        onClicked: {
                            var folder = explorerListView.folderListModel.folder.toString().replace("file://", "") + "/"
                            var name = folder + explorerListView.folderListModel.get(explorerListView.listView.currentIndex, "fileName")
                            var isfolder = explorerListView.folderListModel.isFolder(explorerListView.listView.currentIndex)
                            FileManager.rm(name)
                            popup.close()
                            entryDeleted("file://" + name, isfolder)
                        }
                    }
                }
            }
        ]
        property color containerColor: "#52616D"
        property color textColor: "white"
        property color deleteTextColor: "#f83e29"
        property color lineColor: Qt.darker("#52616D", 1.15)
        property color pressedButtonColor: "white"
    }
    signal entryRenamed(var from, var to, var isdir)
    signal entryDeleted(var name, var isdir)
    property var readOnly: []
    property string dropIcon
    property alias explorerToolBar: explorerToolBar
    property alias explorerListView: explorerListView
}
