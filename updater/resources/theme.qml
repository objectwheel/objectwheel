import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import QtQuick.Extras 1.4

ApplicationWindow {
    visible: false
    width: 360
    height: 600
    title: qsTr("Theme Preview")

    ToolBar {
        id: toolbar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("‹")
            }
            Label {
                text: "Quick Controls"
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

    TabView {
        id: tabBar
        anchors.top: toolbar.bottom
        anchors.left: toolbar.left
        anchors.right: toolbar.right
        anchors.bottom: parent.bottom

        Tab {
            title: "Tab 1"
            Item {
                Slider {
                    id: slider
                    x: 180
                    y: 245
                    width: 165
                    height: 40
                    value: 0.5
                }

                Gauge {
                    id: gauge
                    x: 280
                    y: 290
                    height: 230
                    width: 38
                    value: 6
                    clip: true
                    maximumValue: 10
                    tickmarkStepSize: 2

                    style: GaugeStyle {
                        valueBar: Rectangle {
                            color: "#62A745"
                            implicitWidth: 100
                        }

                        background: Rectangle {
                            color: "#4A5360"
                        }

                        tickmark: Rectangle {
                            color: "#3E4651"
                            implicitWidth: 9
                            implicitHeight: 2
                        }

                        minorTickmark: Rectangle {
                            color: "#667284"
                            implicitWidth: 4
                            implicitHeight: 2
                        }

                        tickmarkLabel: Text {
                            color: "#363D47"
                            text: styleData.value
                        }
                    }
                }

                TextField {
                    id: textField
                    x: 10
                    y: 124
                    width: 165
                    text: qsTr("Text Field")
                }

                ProgressBar {
                    id: progressBar
                    x: 10
                    y: 350
                    width: 165
                    height: 6
                    value: 0.5
                }

                Dial {
                    id: dial
                    x: 180
                    y: 10
                    width: 165
                    height: 140
                }

                DelayButton {
                    x: 180
                    y: 162
                    width: 76
                    height: 76
                    text: "Delay\nButton"
                    delay: 500
                }

                ToggleButton {
                    x: 262
                    y: 162
                    width: 76
                    height: 76
                    text: "Toggle\nButton"
                    checked: true
                }

                ComboBox {
                    id: comboBox
                    x: 10
                    y: 44
                    width: 165
                    model: ["Combo Box", "Item 2", "Item 3", "Item 4"]
                }

                Button {
                    x: 10
                    y: 82
                    width: 165
                    text: qsTr("Button")
                }

                CheckBox {
                    x: 10
                    y: 275
                    text: qsTr("Check Box")
                    checked: true
                }

                CheckBox {
                    x: 10
                    y: 247
                    text: qsTr("Check Box")
                }

                Switch {
                    x: 10
                    y: 315
                }

                Switch {
                    x: 70
                    y: 315
                    checked: true
                }

                GroupBox {
                    id: groupBox
                    x: 10
                    y: 370
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
                    x: 12
                    y: 10
                    width: 165
                    value: 54
                }

                BusyIndicator {
                    x: 10
                    y: 176
                }

                StatusIndicator {
                    x: 70
                    y: 185
                }

                StatusIndicator {
                    x: 120
                    y: 185
                    active: true
                }

            }
        }
        Tab {
            title: "Tab 2"
            Text {
                text: "No Items"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
        Tab {
            title: "Tab 3"
            Text {
                text: "No Items"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}