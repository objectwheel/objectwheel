import QtQuick 2.7
import QtQuick.Extras 1.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

CircularGauge {
    height: 120
    width: 120
    id: circularGauge
    value: 0
    minimumValue: 0
    maximumValue: 100

    Behavior on value {
        SpringAnimation {
            spring: 1.4
            damping: .15
        }
    }

    Text {
        width: parent.width
        anchors.bottom: parent.bottom
        color: "#202020"
        text: "Value (°V)"
        horizontalAlignment: Text.AlignHCenter
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: circularGauge.value = (Math.random() * 100)
    }

    style: CircularGaugeStyle {
        id: style

        function degreesToRadians(degrees) {
            return degrees * (Math.PI / 180)
        }

        background: Canvas {
            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()

                ctx.beginPath()
                ctx.strokeStyle = "#FF3300"
                ctx.lineWidth = outerRadius * 0.02

                ctx.arc(outerRadius, outerRadius,
                        outerRadius - ctx.lineWidth / 2, degreesToRadians(
                            valueToAngle(80) - 90), degreesToRadians(
                            valueToAngle(100) - 90))
                ctx.stroke()
            }
        }

        tickmark: Rectangle {
            visible: styleData.value < 80 || styleData.value % 10 == 0
            implicitWidth: outerRadius * 0.02
            antialiasing: true
            implicitHeight: outerRadius * 0.06
            color: styleData.value >= 80 ? "#FF3300" : "#202020"
        }

        minorTickmark: Rectangle {
            visible: styleData.value < 80
            implicitWidth: outerRadius * 0.01
            antialiasing: true
            implicitHeight: outerRadius * 0.03
            color: "#202020"
        }

        tickmarkLabel: Text {
            font.pixelSize: Math.max(6, outerRadius * 0.1)
            text: styleData.value
            color: styleData.value >= 80 ? "#FF3300" : "#202020"
            antialiasing: true
        }

        foreground: Item {
            Rectangle {
                width: outerRadius * 0.2
                height: width
                radius: width / 2
                color: "#202020"
                anchors.centerIn: parent
            }
        }
    }
}
