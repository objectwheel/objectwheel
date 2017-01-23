import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4 as ControlOlder
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import "filemanager.js" as FileManager
import "fit.js" as Fit

Item {
    id: root
    Rectangle {
        id: base
        anchors.fill: parent
        color: Qt.lighter("#44504e", 1.2)
    }

    Rectangle {
        id: toolBar
        anchors { top: parent.top; left: parent.left; right:parent.right; }
        height: Fit.fit(42)
        color: "#1e8145"

        Row {
            spacing: 0
            anchors.fill: parent
            anchors.margins: Fit.fit(4)

            FancyButton {
                id: backButton
                iconSource: "qrc:///resources/images/file_explorer/back.png"
                anchors.centerIn: parent.Center
                height: parent.height
                width: height
                onClicked: {
                    if (folderList.parentFolder.toString().length >= folderList.rootFolder.toString().length) {
                        if (folderList.folder !== folderList.parentFolder) {
                            swipeView.back()
                            folderList.folder = folderList.parentFolder
                        }
                    }
                }
            }

            FancyButton {
                id: homeButton
                iconSource: "qrc:///resources/images/file_explorer/home.png"
                anchors.centerIn: parent.Center
                height: parent.height
                width: height
                onClicked: {
                    if (folderList.folder != folderList.rootFolder) {
                        swipeView.back();
                        folderList.folder = folderList.rootFolder
                    }
                }
            }

            FancyButton {
                id: newFolderButton
                iconSource: "qrc:///resources/images/file_explorer/new_folder_icon.png"
                anchors.centerIn: parent.Center
                height: parent.height
                width: height
                onClicked: {
                    popup.placeHolderText = "Folder name"
                    popup.textField.text = ""
                    popup.color = "#a4db37"
                    popup.show(x + width/2.0, y + height)
                }
            }

            FancyButton {
                id: newFileButton
                iconSource: "qrc:///resources/images/file_explorer/new_file_icon.png"
                anchors.centerIn: parent.Center
                height: parent.height
                width: height
                onClicked: {
                    popup.placeHolderText = "File name"
                    popup.textField.text = ""
                    popup.color = "#ca3d33"
                    popup.show(x + width/2.0, y + height)
                }
            }
        }
    }

    ShadowFactory {
        targets: [toolBar]
        places: [Item.Bottom]
    }

    Item {
        id: swipeView
        anchors.margins: Fit.fit(5)
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; top: toolBar.bottom; }
        property int animDuration: 200
        property int easingCurve: Easing.OutExpo
        FolderList {
            id: folderList
            width: parent.width
            height: parent.height
            showDirsFirst : true
            showDotAndDotDot : false
            highlightMoveDuration: 80
            highlightColor: Qt.darker(base.color, 1.25)
            onSwipeCall: {
                swipeView.next()
            }
            onCurrentItemChanged: {
                selectionChanged(folderList.folder.toString() + "/" + folderList.itemAt(folderList.currentIndex),
                                 folderList.isDir(folderList.currentIndex))
            }
        }

        FolderList {
            id: folderList2
            x: -width-swipeView.anchors.margins
            width: parent.width
            height: parent.height
            showDirsFirst : true
            showDotAndDotDot : false
            rootFolder: folderList.rootFolder
            highlightMoveDuration: 80
            highlightColor: Qt.darker(base.color, 1.25)
            onSwipeCall: {
                swipeView.next()
            }
        }


        NumberAnimation {
            id: nextAnim1
            target: folderList
            property: "x"
            duration: swipeView.animDuration
            easing.type: swipeView.easingCurve
            from: folderList.width + swipeView.anchors.margins
            to: 0
        }

        NumberAnimation {
            id: nextAnim2
            target: folderList2
            property: "x"
            duration: swipeView.animDuration
            easing.type: swipeView.easingCurve
            from: 0
            to: -folderList2.width - swipeView.anchors.margins
        }

        NumberAnimation {
            id: backAnim2
            target: folderList
            property: "x"
            duration: swipeView.animDuration
            easing.type: swipeView.easingCurve
            from: -folderList.width - swipeView.anchors.margins
            to: 0
        }

        NumberAnimation {
            id: backAnim1
            target: folderList2
            property: "x"
            duration: swipeView.animDuration
            easing.type: swipeView.easingCurve
            from: 0
            to: folderList2.width + swipeView.anchors.margins
        }

        function next() {
            folderList2.folder = folderList.folder
            folderList2.currentIndex = folderList.currentIndex
            folderList.x = folderList.width + swipeView.anchors.margins
            folderList2.x = 0
            nextAnim1.start()
            nextAnim2.start()
        }

        function back() {
            folderList2.folder = folderList.folder
            folderList2.currentIndex = folderList.currentIndex
            folderList.x = -folderList.width - swipeView.anchors.margins
            folderList2.x = 0
            backAnim1.start()
            backAnim2.start()
        }
    }

    PopupWindow {
        property string placeHolderText
        signal buttonClicked(var which);
        property alias textField: textField
        id: popup
        width: Fit.fit(160)
        height: Fit.fit(60)
        focus: true
        modal: true

        contentItem: Item {
            id: popupContent
            anchors.fill: parent
            anchors {leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); bottomMargin: Fit.fit(10); topMargin: Fit.fit(20)}
            ControlOlder.TextField {
                id: textField
                placeholderText: popup.placeHolderText
                style: TextFieldStyle {
                    placeholderTextColor: "gray"
                    textColor: "black"
                    background: Rectangle {
                        radius: Fit.fit(3)
                        implicitWidth: width
                        implicitHeight: height
                        border.color: Qt.darker(popup.color, 1.2)
                        border.width: Fit.fit(1)
                        color: "white"
                    }
                }
                Keys.onReturnPressed : button.clicked()
                anchors {left:parent.left;top:parent.top;bottom:parent.bottom;
                    right:button.left;rightMargin:Fit.fit(5)}
            }
            FancyButton {
                id: button
                width: height
                iconSource: "qrc:///resources/images/file_explorer/ok.png"
                anchors {top:parent.top; bottom:parent.bottom; right:parent.right }
                onClicked: {
                    if (textField.placeholderText.indexOf("File") >= 0) {
                        popup.buttonClicked("file")
                    } else if (textField.placeholderText.indexOf("Folder") >= 0) {
                        popup.buttonClicked("folder")
                    } else if (textField.placeholderText.indexOf("Edit") >= 0) {
                        popup.buttonClicked("edit")
                    }
                    popup.close()
                }
            }
        }
    }

    PopupWindow {
        property string placeHolderText
        signal buttonClicked(var which);

        id: editPopup
        width: Fit.fit(80)
        height: Fit.fit(60)
        focus: true
        color: "#8f44ad"
        modal: true

        contentItem: Item {
            id: popupContent2
            anchors.fill: parent
            anchors {leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); bottomMargin: Fit.fit(10); topMargin: Fit.fit(20)}

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                FancyButton {
                    id: editButton
                    height: popupContent2.height
                    width: height
                    iconSource: "qrc:///resources/images/file_explorer/edit_icon.png"
                    onClicked: {
                        editPopup.close()
                        popup.placeHolderText = "Edit name"
                        popup.textField.text = folderList.itemAt(folderList.currentIndex)
                        popup.show(editPopup.xCord, editPopup.yCord)
                    }
                }
                FancyButton {
                    id: removeButton
                    height: popupContent2.height
                    width: height
                    iconSource: "qrc:///resources/images/file_explorer/remove_icon.png"
                    onClicked: {
                        FileManager.rm(folderList.folder.toString().replace("file://", "") + "/" +folderList.itemAt(folderList.currentIndex))
                        entryRemoved(folderList.folder.toString() + "/" + folderList.itemAt(folderList.currentIndex),
                                    folderList.isDir(folderList.currentIndex))
                        editPopup.close()
                    }
                }
            }
        }

        onVisibleChanged: {
            if (visible) {
                textField.text = ""
            }
        }
    }

    Component.onCompleted: {
        popup.buttonClicked.connect(function(which) {
            if (textField.text == "") return
            var newName = folderList.folder.toString().replace("file://", "") + "/" + textField.text
            if (which === "folder") {
                FileManager.mkdir(newName)
            } else if (which === "file") {
                FileManager.mkfile(newName)
            } else if (which === "edit") {
                FileManager.mv(folderList.folder.toString().replace("file://", "") + "/" +folderList.itemAt(folderList.currentIndex),newName)
                entryEdited(folderList.folder.toString() + "/" + folderList.itemAt(folderList.currentIndex), newName,
                            folderList.isDir(folderList.currentIndex))
            }
        });

        folderList.pressAndHold.connect(function(x, y) {
            for (var i=0; i<readOnly.length; i++) {
                if (readOnly[i] === folderList.itemAt(folderList.currentIndex))
                    return;
            }
            var pt = root.mapFromItem(folderList.currentItem, x, y)
            editPopup.show(pt.x, pt.y)
        })
    }

    signal selectionChanged(url urlval, bool isdir)
    signal entryRemoved(url urlval, bool isdir)
    signal entryEdited(url from, url to, bool isdir)
    property var readOnly: []
    property alias folder : folderList.folder
    property alias rootFolder : folderList.rootFolder
    property alias currentIndex: folderList.currentIndex
    property alias count : folderList.count
    function itemAt(index) {
        return folderList.itemAt(index)
    }
}
