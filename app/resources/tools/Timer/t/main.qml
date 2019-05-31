import QtQuick 2.0

Timer {
    id: timer
    running: true
    onTriggered: {
        console.log("Timer triggered!")
    }
}
