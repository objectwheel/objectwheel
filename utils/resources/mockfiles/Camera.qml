import QtQml 2.12
import QtMultimedia 5.12

QtObject {
    property Camera.CaptureMode captureMode : Camera.CaptureStillImage
    property Camera.Position position : Camera.UnspecifiedPosition
    property real digitalZoom : 1
    property real opticalZoom : 1
    property var viewfinder : QtObject {
        property size resolution
        property real minimumFrameRate : 1
        property real maximumFrameRate : 30
    }
}
