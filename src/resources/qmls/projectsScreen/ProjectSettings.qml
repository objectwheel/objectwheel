import QtQuick 2.7
import QtQuick.Dialogs 1.2 as Dgl
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.0
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller

Item {
    id: root
    Rectangle {
        id: title
        color: "#25000000"
        anchors { top: parent.top; left: parent.left; right: parent.right;}
        height: Fit.fit(40)
        Image {
            id: projectsettingsTextImage
            source: "qrc:///resources/images/projectsettings.png"
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            fillMode: Image.PreserveAspectFit
            height: Fit.fit(16)
        }
        Item {
            id: btnCancelOutCont
            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: Fit.fit(10) }
            width: Fit.fit(80)
            height: Fit.fit(28)
            Item {
                id: btnCancelInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnCancel
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#EA8622" }
                        GradientStop { position: 1.0; color: "#DC521A" }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Text {
                            text: "Cancel"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnCancelImg.height
                            font.pixelSize: Fit.fit(13)
                        }
                        Image {
                            id: btnCancelImg
                            source: "qrc:///resources/images/cancel.png"
                            height: btnCancel.height - Fit.fit(14)
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                    signal clicked();
                    function glow() {
                        btnCancelOverlay.color = "#30000000"
                    }
                    function unglow() {
                        btnCancelOverlay.color = "#00ffffff"
                    }
                }
                ColorOverlay {
                    id: btnCancelOverlay
                    anchors.fill: btnCancel
                    source: btnCancel
                    color: "#00ffffff"
                }
            }
            Rectangle {
                id: btnCancelMask;
                anchors.fill: parent;
                radius: Fit.fit(6);
                Rectangle {
                    height: parent.height
                    width: parent.width / 2
                    anchors.right: parent.right
                    radius: Fit.fit(2)
                }
                visible: false;
            }
            OpacityMask {
                id: btnCancelOpMask
                visible: false
                anchors.fill: btnCancelInCont
                source: btnCancelInCont
                maskSource: btnCancelMask
            }
            DropShadow {
                anchors.fill: btnCancelOpMask
                horizontalOffset: 0
                verticalOffset: Fit.fit(1)
                radius: 3
                samples: 15
                color: "#30000000"
                source: btnCancelOpMask
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: btnCancel.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onPressed: btnCancel.glow()
                onReleased: btnCancel.unglow()
                onClicked: btnCancel.enabled ? btnCancel.clicked() : 0
            }
        }
        Item {
            id: btnOkOutCont
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: Fit.fit(10) }
            width: Fit.fit(80)
            height: Fit.fit(28)
            Item {
                id: btnOkInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnOk
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#8bbb56" }
                        GradientStop { position: 1.0; color: "#6d9f45" }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Image {
                            id: btnOkImg
                            source: "qrc:///resources/images/ok.png"
                            height: btnOk.height - Fit.fit(14)
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            text: "Ok"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnOkImg.height
                            font.pixelSize: Fit.fit(13)
                        }
                    }
                    signal clicked();
                    function glow() {
                        btnOkOverlay.color = "#30000000"
                    }
                    function unglow() {
                        btnOkOverlay.color = "#00ffffff"
                    }
                }
                ColorOverlay {
                    id: btnOkOverlay
                    anchors.fill: btnOk
                    source: btnOk
                    color: "#00ffffff"
                }
            }
            Rectangle {
                id: btnOkMask;
                anchors.fill: parent;
                radius: Fit.fit(6);
                Rectangle {
                    height: parent.height
                    width: parent.width / 2
                    anchors.left: parent.left
                    radius: Fit.fit(2)
                }
                visible: false;
            }
            OpacityMask {
                id: btnOkOpMask
                visible: false
                anchors.fill: btnOkInCont
                source: btnOkInCont
                maskSource: btnOkMask
            }
            DropShadow {
                anchors.fill: btnOkOpMask
                horizontalOffset: 0
                verticalOffset: Fit.fit(1)
                radius: 3
                samples: 15
                color: "#30000000"
                source: btnOkOpMask
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: btnOk.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onPressed: btnOk.glow()
                onReleased: btnOk.unglow()
                onClicked: btnOk.enabled ? btnOk.clicked() : 0
            }
        }
        Rectangle {
            id: titleLine
            anchors.bottom: parent.bottom
            width: parent.width
            height: Fit.fit(1)
            color: "#25000000"
        }
    }
    Flickable {
        id: flickable
        clip: true
        anchors { top: title.bottom; bottom: parent.bottom; right: parent.right; left:parent.left; }
        ScrollBar.vertical: ScrollBar { }
        contentWidth: width
        contentHeight: projectnameContainer.height + descriptionContainer.height +
                       ownerContainer.height + sizeContainer.height +
                       mfDateContainer.height + crDateContainer.height +
                       delProjectContainer.height + imExProjectContainer.height
        Column {
            anchors.fill: parent
            Rectangle {
                id: projectnameContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: projectnameTitle
                        text: "Project Name"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: projectnameTextInput
                        clip: true
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: descriptionContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: descriptionTitle
                        text: "Description"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: descriptionTextInput
                        clip: true
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: ownerContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: ownerTitle
                        text: "Owner"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: ownerText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        font.underline: true
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: crDateContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: crDateTitle
                        text: "Creation Date"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: crDateText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        font.underline: true
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: mfDateContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: mfDateTitle
                        text: "Last Modification Date"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: mfDateText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        font.underline: true
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: sizeContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: sizeTitle
                        text: "Size On Disk"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: sizeText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        font.underline: true
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: imExProjectContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: imExProjectTitle
                        text: "Import/Export Project"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true; }
                    Item {
                        id: btnImExOutCont
                        width: Fit.fit(160)
                        height: Fit.fit(28)
                        Item {
                            id: btnImExInCont
                            anchors.fill: parent
                            visible: false

                            Rectangle {
                                id: btnImExLeft
                                anchors { left: parent.left; top: parent.top; bottom: parent.bottom}
                                width: parent.width / 2.0
                                visible: false
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: "#C55DFF" }
                                    GradientStop { position: 1.0; color: "#A336FF" }
                                }
                                Row {
                                    anchors.centerIn: parent
                                    spacing: Fit.fit(6)
                                    Text {
                                        text: "Import"
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        height: btnImExImgLeft.height
                                        font.pixelSize: Fit.fit(13)
                                    }
                                    Image {
                                        id: btnImExImgLeft
                                        source: "qrc:///resources/images/load.png"
                                        height: btnImExLeft.height - Fit.fit(14)
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }
                                signal clicked();
                                function glow() {
                                    btnImExLeftOverlay.color = "#30000000"
                                }
                                function unglow() {
                                    btnImExLeftOverlay.color = "#00ffffff"
                                }
                            }
                            Rectangle {
                                id: btnImExRight
                                anchors { right: parent.right; top: parent.top; bottom: parent.bottom}
                                width: parent.width / 2.0
                                visible: false
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: "#5bc5f8" }
                                    GradientStop { position: 1.0; color: "#2491f9" }
                                }
                                Row {
                                    anchors.centerIn: parent
                                    spacing: Fit.fit(6)
                                    Image {
                                        id: btnImExRightImg
                                        source: "qrc:///resources/images/unload.png"
                                        height: btnImExRight.height - Fit.fit(14)
                                        fillMode: Image.PreserveAspectFit
                                    }
                                    Text {
                                        text: "Export"
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        height: btnImExRightImg.height
                                        font.pixelSize: Fit.fit(13)
                                    }
                                }
                                signal clicked();
                                function glow() {
                                    btnImExRightOverlay.color = "#30000000"
                                }
                                function unglow() {
                                    btnImExRightOverlay.color = "#00ffffff"
                                }
                            }

                            ColorOverlay {
                                id: btnImExRightOverlay
                                anchors.fill: btnImExRight
                                source: btnImExRight
                                color: "#00ffffff"
                            }
                            ColorOverlay {
                                id: btnImExLeftOverlay
                                anchors.fill: btnImExLeft
                                source: btnImExLeft
                                color: "#00ffffff"
                            }
                        }
                        Rectangle {
                            id: btnImExMask;
                            anchors.fill: parent;
                            radius: Fit.fit(6);
                            visible: false;
                        }
                        OpacityMask {
                            id: btnImExOpMask
                            visible: false
                            anchors.fill: btnImExInCont
                            source: btnImExInCont
                            maskSource: btnImExMask
                        }
                        DropShadow {
                            anchors.fill: btnImExOpMask
                            horizontalOffset: 0
                            verticalOffset: Fit.fit(1)
                            radius: 3
                            samples: 15
                            color: "#30000000"
                            source: btnImExOpMask
                        }
                        Rectangle {
                            width: Fit.fit(1)
                            anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; bottom: parent.bottom}
                            color: "#30545454"
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: {
                                if (mouseX < parent.width / 2.0 && btnImExLeft.enabled) Qt.PointingHandCursor
                                else if (mouseX >= parent.width / 2.0 && btnImExRight.enabled) Qt.PointingHandCursor
                                else Qt.ArrowCursor
                            }
                            onPressed: {
                                if (mouse.x < parent.width / 2.0 && btnImExLeft.enabled) btnImExLeft.glow()
                                else if (mouse.x >= parent.width / 2.0 && btnImExRight.enabled) btnImExRight.glow()
                            }
                            onReleased: {
                                btnImExLeft.unglow(); btnImExRight.unglow()
                            }
                            onClicked: {
                                if (mouse.x < parent.width / 2.0 && btnImExLeft.enabled) btnImExLeft.clicked()
                                else if (mouse.x >= parent.width / 2.0 && btnImExRight.enabled) btnImExRight.clicked()
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: delProjectContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: delProjectTitle
                        text: "Delete Project"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true; }
                    Item {
                        id: btnDelProjectOutCont
                        width: Fit.fit(80)
                        height: Fit.fit(28)
                        Item {
                            id: btnDelProjectInCont
                            anchors.fill: parent
                            visible: false
                            Rectangle {
                                id: btnDelProject
                                anchors.fill: parent
                                visible: false
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: "#f2474a" }
                                    GradientStop { position: 1.0; color: "#CB2023" }
                                }
                                Row {
                                    anchors.centerIn: parent
                                    spacing: Fit.fit(6)
                                    Image {
                                        id: btnDelProjectImg
                                        source: "qrc:///resources/images/cancel.png"
                                        height: btnDelProject.height - Fit.fit(14)
                                        fillMode: Image.PreserveAspectFit
                                    }
                                    Text {
                                        text: "Delete"
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        height: btnDelProjectImg.height
                                        font.pixelSize: Fit.fit(13)
                                    }
                                }
                                signal clicked();
                                function glow() {
                                    btnDelProjectOverlay.color = "#30000000"
                                }
                                function unglow() {
                                    btnDelProjectOverlay.color = "#00ffffff"
                                }
                            }
                            ColorOverlay {
                                id: btnDelProjectOverlay
                                anchors.fill: btnDelProject
                                source: btnDelProject
                                color: "#00ffffff"
                            }
                        }
                        Rectangle {
                            id: btnDelProjectMask;
                            anchors.fill: parent;
                            radius: Fit.fit(6);
                            visible: false;
                        }
                        OpacityMask {
                            id: btnDelProjectOpMask
                            visible: false
                            anchors.fill: btnDelProjectInCont
                            source: btnDelProjectInCont
                            maskSource: btnDelProjectMask
                        }
                        DropShadow {
                            anchors.fill: btnDelProjectOpMask
                            horizontalOffset: 0
                            verticalOffset: Fit.fit(1)
                            radius: 3
                            samples: 15
                            color: "#30000000"
                            source: btnDelProjectOpMask
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: btnDelProject.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onPressed: btnDelProject.glow()
                            onReleased: btnDelProject.unglow()
                            onClicked: btnDelProject.enabled ? deleteProjectMessageDialog.visible = true : 0
                        }
                    }
                }
            }
        }
    }
    Item {
        id: warning
        y: -height - Fit.fit(10)
        width: parent.width / 2.4
        height: Fit.fit(55)
        anchors.horizontalCenter: parent.horizontalCenter
        Rectangle {
            id: base
            anchors.fill: parent
            radius: Fit.fit(7)
            visible: false
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter("#C0C3C5",1.03) }
                GradientStop { position: 1.0; color: Qt.darker("#C0C3C5",1.03) }
            }
            border.color: "#B0AFB0"
        }
        DropShadow {
            anchors.fill: parent
            horizontalOffset: 0
            verticalOffset: Fit.fit(1)
            radius: (2.0)
            samples: (17)
            color: "#40000000"
            source: base
        }
        Text {
            font.pixelSize: Fit.fit(13)
            text: "Please fill all the fields."
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            y: parent.height/2.0 -contentHeight/2.0 + Fit.fit(4)
            color: "#302E30"

            Text {
                font.pixelSize: Fit.fit(16)
                font.bold: true
                text: "*"
                anchors.verticalCenter: parent.verticalCenter
                x: parent.width/2.0 - parent.contentWidth/2.0 - Fit.fit(10)
                color: "#bb3333"
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: 350
                easing.type: Easing.InExpo
            }
        }
        function show() {
            y = -Fit.fit(10)
            DelayCaller.delayCall(4000, function() {
                y = -height - Fit.fit(10)
            })
        }
    }
    Dgl.MessageDialog {
        id: deleteProjectMessageDialog
        title: "Delete Project"
        text: "This will delete entire project. Are you sure?"
         standardButtons: Dgl.StandardButton.Yes | Dgl.StandardButton.No
         icon: Dgl.StandardIcon.Warning
         onYes: btnDelProject.clicked()
        // BUG: on IOS
    }
    property alias warning: warning
    property alias projectnameTextInput: projectnameTextInput
    property alias descriptionTextInput: descriptionTextInput
    property alias ownerText: ownerText
    property alias crDateText: crDateText
    property alias mfDateText: mfDateText
    property alias sizeText: sizeText
    property alias btnDelete: btnDelProject
    property alias btnImport: btnImExLeft
    property alias btnExport: btnImExRight
    property alias btnOk: btnOk
    property alias btnCancel: btnCancel
}
