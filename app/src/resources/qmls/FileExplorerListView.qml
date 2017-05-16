import QtQuick 2.7
import QtQuick.Controls 2.1
import com.objectwheel.components 1.0
import "fit.js" as Fit
import "filemanager.js" as FileManager

Item {
    id: root
    ListView {
        id: listView
        x:0; y:0;
        width: parent.width
        height: parent.height
        model: folderListModel
        delegate: folderListDelegate
        highlightFollowsCurrentItem: false
        highlight: highlight
        ScrollBar.vertical: ScrollBar { }
        currentIndex: -1
    }
    Text {
        id: emptyText
        anchors.fill: listView
        text: "Empty"
        color: "#50ffffff"
        visible: folderListModel.count === 0
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    FolderListModel {
        id: folderListModel
    }
    Component {
        id: imageComponent
        Image {
            x: 0; y: 0;
            width: listView.width
            height: listView.height
        }
    }
    Component {
        id: folderListDelegate
        Item {
            height: elementHeight
            width: parent.width
            Row {
                anchors.fill: parent
                anchors.margins: Fit.fit(4)
                spacing: Fit.fit(5)
                Image {
                    source: fileIsDir ? dirIcon : d.determineIcon(fileSuffix)
                    height: parent.height
                    width: height
                    sourceSize: Qt.size(width, height)
                    anchors.verticalCenter: parent.verticalCenter
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        width: textWidth
                        elide: Text.ElideRight
                        color: textColor
                        text: fileName
                        font.bold: true
                        font.pixelSize: textSize
                    }
                    Text {
                        width: textWidth
                        elide: Text.ElideRight
                        color: textColor
                        text: fileModified.toLocaleString(Qt.locale(), "yyyy.MM.dd hh:mm")
                        font.pixelSize: fileIsDir ? (textSize - 1) : (textSize - 4)
                    }
                    Text {
                        width: textWidth
                        elide: Text.ElideRight
                        color: textColor
                        visible: !fileIsDir
                        text: fileIsDir ? "" : ( (fileSize>=1024)  ? (Math.ceil(fileSize/1024.0) + " kB") : (fileSize + " bytes"))
                        font.pixelSize: (textSize - 4)
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    listView.currentIndex = index
                    if (fileIsDir && mouse.button !== Qt.RightButton) {
                        var newFile = folderListModel.get(index, "fileURL").toString()
                        go(newFile, true)
                    }
                    itemClicked(mouse, index)
                }
                onPressAndHold: {
                    listView.currentIndex = index
                    itemPressedAndHeld(mouse, index)
                }
            }
        }
    }
    Component {
        id: highlight
        Rectangle {
            width: listView.width
            height: elementHeight
            color: "#50000000"
            radius: Fit.fit(3)
            visible: listView.currentItem.visible
            y: listView.currentItem.y
        }
    }
    NumberAnimation {
        id: listViewAnimation
        target: listView
        property: "x"
        duration: d.duration
        easing.type: d.curve
    }
    NumberAnimation {
        id: imageAnimation
        target: d.image
        property: "x"
        duration: d.duration
        easing.type: d.curve
        onStopped: {
            if (d.image !== null) {
                d.image.destroy()
                d.image = null
            }
        }
    }
    QtObject {
        id: d
        function determineIcon(suffix) {
            if (FileManager.stype(suffix) === "img") {
                return imageIcon
            } else if (FileManager.stype(suffix) === "txt") {
                return textIcon
            } else if (FileManager.stype(suffix) === "js") {
                return jsIcon
            } else if (FileManager.stype(suffix) === "qml") {
                return qmlIcon
            } else if (FileManager.stype(suffix) === "bin") {
                return binIcon
            }
        }
        property var image: null
        property int duration: 500
        property int curve: Easing.OutExpo
    }
    function go(url, fromRight) {
        listView.grabToImage(function(result) {
            folderListModel.folder = url
            imageAnimation.stop()
            listViewAnimation.stop()
            d.image = imageComponent.createObject(root)
            d.image.source = result.url
            imageAnimation.from = 0
            listViewAnimation.from = fromRight ? listView.width : -listView.width
            imageAnimation.to = fromRight ? -d.image.width : d.image.width
            listViewAnimation.to = 0
            imageAnimation.start()
            listViewAnimation.start()
        })
    }
    function goBack() {
        if (folderListModel.parentFolder.toString().length >= folderListModel.rootFolder.toString().length) {
            if (folderListModel.folder !== folderListModel.parentFolder) {
                go(folderListModel.parentFolder, false)
            }
        }
    }
    function goRoot() {
        if (folderListModel.rootFolder !== folderListModel.folder) {
            go(folderListModel.rootFolder, false)
        }
    }

    signal itemClicked(var mouse, var index)
    signal itemPressedAndHeld(var mouse, var index)
    property alias listView: listView
    property alias folderListModel: folderListModel
    property real elementHeight: Fit.fit(40)
    property real textSize: Fit.fit(12)
    property real textWidth: Fit.fit(120)
    property color textColor
    property string imageIcon
    property string qmlIcon
    property string jsIcon
    property string textIcon
    property string binIcon
    property string dirIcon
}
