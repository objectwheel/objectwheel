import QtQuick 2.7
import QtGraphicalEffects 1.0
import "fit.js" as Fit

Item {
    property var targets: []
    property var places:  []
    property int offset: 1 * dpi
    property real radius: 4.0
    property int samples: 15
    property color color: "#40000000"

    id: root

    Component {
        id: shadowComponent
        DropShadow {
            radius: root.radius
            samples: root.samples
            color: root.color
            horizontalOffset: 0
            verticalOffset: 0
        }
    }

    onTargetsChanged: {
        if (targets.length > 0 && places.length == targets.length) {
            for(var i = 0; i < targets.length; i++) {
                if (places[i] === Item.Top) {
                    shadowComponent.createObject(targets[i].parent, {"anchors.fill": targets[i], "source": targets[i],
                                                     "verticalOffset": - offset})
                } else if (places[i] === Item.Bottom) {
                    shadowComponent.createObject(targets[i].parent, {"anchors.fill": targets[i], "source": targets[i],
                                                     "verticalOffset": offset})
                } else if (places[i] === Item.Right) {
                    shadowComponent.createObject(targets[i].parent, {"anchors.fill": targets[i], "source": targets[i],
                                                     "horizontalOffset": offset})
                } else {
                    shadowComponent.createObject(targets[i].parent, {"anchors.fill": targets[i], "source": targets[i],
                                                     "horizontalOffset": - offset})
                }
            }
        }
    }
}
