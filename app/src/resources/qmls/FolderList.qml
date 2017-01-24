import QtQuick 2.7
import Qt.labs.folderlistmodel 2.1
import QtQuick.Controls 2.0
import "delaycaller.js" as DelayCaller
import "fit.js" as Fit
import "filemanager.js" as FileManager

Item {
    id: root

    ListView {
        id: fileList
        anchors.fill: parent
        clip: true
        // keyNavigationEnabled : false
        highlight: Rectangle { color: highlightColor; radius: Fit.fit(3)}
        highlightMoveDuration: root.highlightMoveDuration
        focus: true
        model: folderModel
        delegate: fileDelegate
        ScrollBar.vertical: ScrollBar { }

        Component {
            id: fileDelegate

            Item {
                height: Fit.fit(40)
                width: parent.width;
                visible: {
                    for (var i=0; i<hiddenExtensions.length; i++) {
                        if (hiddenExtensions[i] === fileSuffix) {
                            return false
                        }
                    }
                    return true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        fileList.currentIndex = index
                        if (fileIsDir) {
                            DelayCaller.delayCall(highlightMoveDuration + 10, function() {
                                root.swipeCall()
                                root.folder = "file://" + filePath
                            })
                        }
                    }
                    onPressAndHold: {
                        var x = mouse.x
                        var y = mouse.y
                        fileList.currentIndex = index
                        DelayCaller.delayCall(highlightMoveDuration + 10, function() {
                            root.pressAndHold(x, y)
                        })
                    }
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: Fit.fit(4)
                    spacing: Fit.fit(5)
                    Image {
                        source: fileIsDir ? dirIcon : determineIcon(fileSuffix)
                        height: parent.height
                        width: height
                        sourceSize: Qt.size(width, height)
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            width: Fit.fit(120);
                            elide: Text.ElideRight;
                            color: "white";
                            text: fileName
                            font.bold: true
                            font.pixelSize: Fit.fit(12)
                        }
                        Text {
                            width: Fit.fit(120);
                            elide: Text.ElideRight;
                            color: "white";
                            text: fileModified.toLocaleString(Qt.locale(), "yyyy.MM.dd hh:mm")
                            font.pixelSize: fileIsDir ? Fit.fit(11) : Fit.fit(8)
                        }
                        Text {
                            width: Fit.fit(120);
                            elide: Text.ElideRight;
                            color: "white";
                            visible: !fileIsDir
                            text: fileIsDir ? "" : ( (fileSize>=1024)  ? (Math.ceil(fileSize/1024.0) + " kB") : (fileSize + " bytes"))
                            font.pixelSize: Fit.fit(8)
                        }
                    }
                }

            }
        }

        FolderListModel {
            id: folderModel
        }
    }


    onCountChanged: {
        if (count <= 0) {
            emptyText.visible = true
        } else {
            emptyText.visible = false
        }
    }

    Text {
        id: emptyText
        anchors.fill: parent
        text: "Empty"
        color: "#cccccc"
        visible: false
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    property color highlightColor
    property int highlightMoveDuration: 200
    property alias rootFolder : folderModel.rootFolder
    property alias folder : folderModel.folder
    property alias parentFolder : folderModel.parentFolder
    property alias showDirsFirst:  folderModel.showDirsFirst
    property alias showDotAndDotDot:  folderModel.showDotAndDotDot
    property alias nameFilters : folderModel.nameFilters
    property alias count: fileList.count
    property alias currentIndex: fileList.currentIndex
    property alias currentItem: fileList.currentItem
    property var hiddenExtensions: []
    property string imageIcon: "qrc:///resources/images/file_explorer/image_icon.svg"
    property string qmlIcon: "qrc:///resources/images/file_explorer/qml_icon.svg"
    property string jsIcon: "qrc:///resources/images/file_explorer/js_icon.svg"
    property string textIcon: "qrc:///resources/images/file_explorer/text_icon.png"
    property string fileIcon: "qrc:///resources/images/file_explorer/file_icon.svg"
    property string dirIcon: "qrc:///resources/images/file_explorer/dir_icon.png"
    signal pressAndHold(var x, var y)
    signal swipeCall()

    function itemAt(index) {
        return folderModel.get(index, "fileName")
    }
    function isDir(index) {
        return folderModel.isFolder(index)
    }
    function determineIcon(suffix) {
        if (FileManager.stype(suffix) === "img") {
            return imageIcon;
        } else if (FileManager.stype(suffix) === "txt") {
            return textIcon;
        } else if (FileManager.stype(suffix) === "js") {
            return jsIcon;
        } else if (FileManager.stype(suffix) === "qml") {
            return qmlIcon;
        } else if (FileManager.stype(suffix) === "bin") {
            return fileIcon;
        }
    }
}
