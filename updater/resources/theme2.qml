import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: false
    width: 360
    height: 600
    title: qsTr("Theme Preview")

    Label {
        text: "My Application"
        elide: Label.ElideRight
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        Layout.fillWidth: true
    }

    header: ToolBar {
        id: toolbar
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("‹")
            }
            Label {
                text: "Quick Controls 2"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            ToolButton {
                text: qsTr("⋮")
            }
        }
    }

    TabBar {
        id: tabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 50

        TabButton {
            text: qsTr("Tab 1")
        }
        TabButton {
            text: qsTr("Tab 2")
        }
        TabButton {
            text: qsTr("Tab 3")
        }
    }

    Slider {
        id: slider
        x: 180
        y: 358
        width: 165
        height: 40
        value: 0.5
    }

    RangeSlider {
        id: rangeSlider
        x: 180
        y: 309
        width: 165
        height: 40
        first.value: 0.25
        second.value: 0.75
    }

    TextField {
        id: textField
        x: 10
        y: 246
        width: 165
        text: qsTr("Text Field")
    }

    Switch {
        id: switch1
        x: 10
        y: 474
        width: 165
        text: qsTr("Switch")
        checked: true
    }

    ProgressBar {
        id: progressBar
        x: 10
        y: 531
        width: 165
        height: 6
        value: 0.5
    }

    Dial {
        id: dial
        x: 180
        y: 68
        width: 165
        height: 160
    }

    ComboBox {
        id: comboBox
        x: 10
        y: 124
        width: 165
        height: 45
        model: ["Combo Box", "Item 2", "Item 3", "Item 4"]
    }

    CheckBox {
        id: checkBox
        x: 10
        y: 355
        width: 165
        text: qsTr("Check Box")
        checked: true
    }

    Button {
        id: button
        x: 10
        y: 182
        width: 165
        height: 45
        text: qsTr("Button")
    }

    CheckBox {
        id: checkBox2
        x: 10
        y: 307
        width: 165
        text: qsTr("Check Box")
    }

    Switch {
        id: switch2
        x: 10
        y: 421
        width: 165
        text: qsTr("Switch")
    }

    GroupBox {
        id: groupBox
        x: 180
        y: 400
        width: 165
        height: 137
        title: qsTr("Group Box")

        ColumnLayout {
            anchors.fill: parent

            RadioButton {
                id: radioButton1
                text: qsTr("Radio Button")
            }

            RadioButton {
                id: radioButton
                text: qsTr("Radio Button")
                checked: true
            }
        }

    }

    SpinBox {
        id: spinBox
        x: 10
        y: 68
        width: 165
        height: 40
        value: 54
    }

    BusyIndicator {
        id: busyIndicator
        x: 230
        y: 236
    }

}