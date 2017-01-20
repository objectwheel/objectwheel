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
        onSourceChanged: {
            DelayCaller.delayCall(200, function() {
                if (Math.min(sourceSize.width,sourceSize.height) > defaultSize) {
                    if (sourceSize.width > sourceSize.height) {
                        width = defaultSize
                    } else {
                        height = defaultSize
                    }
                }
            })
        }
    }

    PinchArea {
        anchors.fill: parent
        pinch.target: image
        pinch.minimumRotation: -360
        pinch.maximumRotation: 360
        pinch.minimumScale: 0.2
        pinch.maximumScale: 10
        pinch.dragAxis: Pinch.NoDrag
        onSmartZoom: {
            if (pinch.scale > 0) {
                image.scale = Math.min(root.width, root.height) / Math.max(image.sourceSize.width, image.sourceSize.height) * 0.85
            } else {
                image.scale = pinch.previousScale
            }
        }

        MouseArea {
            anchors.fill: parent
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
