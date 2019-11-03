import QtQuick 2.12
import QtPositioning 5.12
import QtLocation 5.12

Map {
    id: map
    width: 200
    height: 200
    zoomLevel: 14
    copyrightsVisible: false
    activeMapType: supportedMapTypes[0]
    center: QtPositioning.coordinate(40.779, 30.403) // Sakarya
    gesture.flickDeceleration: 3000
    gesture.enabled: true
    gesture.acceptedGestures: MapGestureArea.PanGesture
                              | MapGestureArea.FlickGesture
                              | MapGestureArea.PinchGesture
                              | MapGestureArea.RotationGesture
                              | MapGestureArea.TiltGesture
    plugin: Plugin {
        id: mapPlugin
        name: "mapboxgl"
    }
}
