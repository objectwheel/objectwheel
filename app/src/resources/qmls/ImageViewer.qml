import QtQuick 2.7
import QtQuick.Controls 2.1
import "delaycaller.js" as DelayCaller

Item {
    id: root

    Image {
        id: image;
        anchors.centerIn: parent
        antialiasing: true
        fillMode: Image.PreserveAspectFit
        onStatusChanged: {
            if (status === Image.Ready) {
                if (Math.min(sourceSize.width,sourceSize.height) > defaultSize) {
                    if (sourceSize.width > sourceSize.height) {
                        width = defaultSize
                    } else {
                        height = defaultSize
                    }
                }
            }
        }
    }

    PinchArea {
        anchors.fill: parent
        onPinchUpdated: {
            image.scale = pinch.scale
        }
        onPinchFinished: {
            image.width *= pinch.scale
            image.height *= pinch.scale
            image.scale = 1.0
        }

        MouseArea {
            anchors.fill: parent
            preventStealing: true
            cursorShape: Qt.OpenHandCursor
            onWheel: {
                var ratio = image.scale * wheel.angleDelta.y / 120 / 10
                if ((image.scale > 10 && ratio < 0) || (image.scale < 0.2 && ratio > 0) || (image.scale > 0.2 && image.scale < 10))
                    image.scale += ratio
            }
        }
    }

    property real defaultSize: 80
    property alias source: image.source
}
