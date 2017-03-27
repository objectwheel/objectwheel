import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.0

Item {
    Rectangle {
        id: title
        color: "#25000000"
        anchors { top: parent.top; left: parent.left; right: parent.right;}
        height: fit(40)
        Image {
            id: projectsettingsTextImage
            source: "images/projectsettings.png"
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            fillMode: Image.PreserveAspectFit
            height: fit(16)
        }
        Item {
            id: btnCancelOutCont
            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: fit(10) }
            width: fit(80)
            height: fit(28)
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
                        spacing: fit(6)
                        Text {
                            text: "Cancel"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnCancelImg.height
                            font.pixelSize: fit(13)
                        }
                        Image {
                            id: btnCancelImg
                            source: "images/cancel.png"
                            height: btnCancel.height - fit(14)
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
                radius: fit(6);
                Rectangle {
                    height: parent.height
                    width: parent.width / 2
                    anchors.right: parent.right
                    radius: fit(2)
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
                verticalOffset: fit(1)
                radius: fit(6.0)
                samples: fit(14)
                color: "#80000000"
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
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: fit(10) }
            width: fit(80)
            height: fit(28)
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
                        spacing: fit(6)
                        Image {
                            id: btnOkImg
                            source: "images/ok.png"
                            height: btnOk.height - fit(14)
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            text: "Ok"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnOkImg.height
                            font.pixelSize: fit(13)
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
                radius: fit(6);
                Rectangle {
                    height: parent.height
                    width: parent.width / 2
                    anchors.left: parent.left
                    radius: fit(2)
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
                verticalOffset: fit(1)
                radius: fit(6.0)
                samples: fit(14)
                color: "#80000000"
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
            height: fit(1)
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
                       orgnameContainer.height + orgIdentContainer.height +
                       projectIdentContainer.height + crDateContainer.height +
                       ownerContainer.height + sizeContainer.height +
                       mfDateContainer.height + projectVersionContainer.height +
                       delProjectContainer.height + imExProjectContainer.height
        Column {
            anchors.fill: parent
            Rectangle {
                id: projectnameContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: projectnameTitle
                        text: "Project Name"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextInput {
                        id: projectnameTextInput
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: descriptionContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: descriptionTitle
                        text: "Description"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextInput {
                        id: descriptionTextInput
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: orgnameContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: orgnameTitle
                        text: "Organization Name"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextInput {
                        id: orgnameTextInput
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: orgIdentContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: orgIdentTitle
                        text: "Organization Identifier"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextInput {
                        id: orgIdentTextInput
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: projectVersionContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: projectVersionTitle
                        text: "Project Version"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextInput {
                        id: projectVersionTextInput
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: projectIdentContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: projectIdentTitle
                        text: "Project Identifier"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: projectIdentText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: ownerContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: ownerTitle
                        text: "Owner"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: ownerText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: crDateContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: crDateTitle
                        text: "Creation Date"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: crDateText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: mfDateContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: mfDateTitle
                        text: "Last Modification Date"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: mfDateText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: sizeContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: sizeTitle
                        text: "Size On Disk"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Text {
                        id: sizeText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextInput.AlignVCenter
                        horizontalAlignment: TextInput.AlignRight
                        font.pixelSize: fit(13)
                    }
                }
            }
            Rectangle {
                id: imExProjectContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: fit(40)
                color: "#07000000"
                border.color: "#14000000"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: imExProjectTitle
                        text: "Import/Export Project"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true; }
                    Item {
                        id: btnImExOutCont
                        width: fit(160)
                        height: fit(28)
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
                                    spacing: fit(6)
                                    Text {
                                        text: "Import"
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        height: btnImExImgLeft.height
                                        font.pixelSize: fit(13)
                                    }
                                    Image {
                                        id: btnImExImgLeft
                                        source: "images/load.png"
                                        height: btnImExLeft.height - fit(14)
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
                                    spacing: fit(6)
                                    Image {
                                        id: btnImExRightImg
                                        source: "images/unload.png"
                                        height: btnImExRight.height - fit(14)
                                        fillMode: Image.PreserveAspectFit
                                    }
                                    Text {
                                        text: "Export"
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        height: btnImExRightImg.height
                                        font.pixelSize: fit(13)
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
                            radius: fit(6);
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
                            verticalOffset: fit(1)
                            radius: fit(6.0)
                            samples: fit(14)
                            color: "#80000000"
                            source: btnImExOpMask
                        }
                        Rectangle {
                            width: fit(1)
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
                height: fit(40)
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: fit(10); rightMargin: fit(10); }
                    Text {
                        id: delProjectTitle
                        text: "Delete Project"
                        font.pixelSize: fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    Item { Layout.fillWidth: true; Layout.fillHeight: true; }
                    Item {
                        id: btnDelProjectOutCont
                        width: fit(80)
                        height: fit(28)
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
                                    spacing: fit(6)
                                    Image {
                                        id: btnDelProjectImg
                                        source: "images/cancel.png"
                                        height: btnDelProject.height - fit(14)
                                        fillMode: Image.PreserveAspectFit
                                    }
                                    Text {
                                        text: "Delete"
                                        color: "white"
                                        verticalAlignment: Text.AlignVCenter
                                        height: btnDelProjectImg.height
                                        font.pixelSize: fit(13)
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
                            radius: fit(6);
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
                            verticalOffset: fit(1)
                            radius: fit(6.0)
                            samples: fit(14)
                            color: "#80000000"
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
    MessageDialog {
        id: deleteProjectMessageDialog
        title: "Do you want to continue?"
        text: "This will delete the project and its content."
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: btnDelProject.clicked()
        icon: StandardIcon.Warning
    }
    property alias projectnameTextInput: projectnameTextInput;
    property alias descriptionTextInput: descriptionTextInput;
    property alias orgnameTextInput: orgnameTextInput;
    property alias orgIdentTextInput: orgIdentTextInput;
    property alias projectVersionTextInput: projectVersionTextInput;
    property alias projectIdentText: projectIdentText;
    property alias ownerText: ownerText;
    property alias crDateText: crDateText;
    property alias mfDateText: mfDateText;
    property alias sizeText: sizeText
    property alias btnDelete: btnDelProject
    property alias btnImport: btnImExLeft
    property alias btnExport: btnImExRight
    property alias btnOk: btnOk
    property alias btnCancel: btnCancel
    function fit(val) { return val }
}
