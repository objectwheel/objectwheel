import QtQuick 2.7
import QtQuick.Controls 2.1
import "delaycaller.js" as DelayCaller
import "fit.js" as Fit

Item {
    id: root

    Image {
        id: image
        autoTransform: true
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
                errorText.visible = false
            } else if (status === Image.Error) {
                errorText.visible = true
            }
        }
    }
    Item {
        id: errorText
        anchors.fill: root
        visible: false
        clip: true
        Image {
            id: errorImage
            width: Math.min(Fit.fit(64), 128)
            height: width
            x: Math.floor((root.width - width)/2.0)
            y: Math.floor((root.height - height)/2.0)
        }
        Text {
            clip: true
            verticalAlignment: Text.AlignTop
            horizontalAlignment: Text.AlignHCenter
            anchors {bottom:parent.bottom; top:errorImage.bottom}
            width: root.width - Fit.fit(50)
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Unable to load image, please check actual image format with file suffix against possible conflicts."
            wrapMode: Text.WordWrap
            color: "white"
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

    property alias warningImageSource : errorImage.source
    property real defaultSize: 80
    property alias source: image.source
}
