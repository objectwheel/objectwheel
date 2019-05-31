import QtQuick 2.7
import QtQuick.Extras 1.4

StatusIndicator {
    height: 40
    width: 40
    id: statusIndicator
    color: "#992210"

    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: statusIndicator.active = !statusIndicator.active
    }
}
