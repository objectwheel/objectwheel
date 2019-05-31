import QtQuick 2.7
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles 1.4

Gauge {
    id: gauge
    height: 200
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
