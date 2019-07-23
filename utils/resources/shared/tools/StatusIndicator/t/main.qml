import QtQuick 2.7
import QtQuick.Extras 1.4

StatusIndicator {
    id: statusIndicator
    width: 40
    height: 40
    color: "#992210"

    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: statusIndicator.active = !statusIndicator.active
    }
}
