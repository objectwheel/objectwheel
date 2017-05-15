import QtQuick 2.7
import QtGraphicalEffects 1.0
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller

Rectangle {
    id: root
    color: "#30000000"
    opacity: 0
    visible: false
    MouseArea {
        anchors.fill: root
        onClicked: hide()
        visible: root.visible
        enabled: root.visible
    }
    Rectangle {
        id: base
        y: -height-base.radius-Fit.fit(5)
        anchors.horizontalCenter: parent.horizontalCenter
        visible: false
        Rectangle {
            id: line
            color: base.color
            width: parent.width
            height: base.radius+Fit.fit(5)
            visible: false
        }
        DropShadow {
            anchors.fill: line
            horizontalOffset: 0
            verticalOffset: Fit.fit(2)
            radius: (6)
            samples: (18)
            color: "#20000000"
            source: line
        }
        PropertyAnimation {
            id: animationForth
            target: base
            property: "y"
            from: -height-base.radius-Fit.fit(5)
            to: -base.radius-Fit.fit(5)
            duration: 500
            easing.type: Easing.OutExpo
        }
        PropertyAnimation {
            id: animationForth2
            property: "opacity"
            target: root
            from: 0
            to: 1
            duration: 300
        }
        PropertyAnimation {
            id: animationBack
            target: base
            property: "y"
            to: -height-base.radius-Fit.fit(5)
            from: -base.radius-Fit.fit(5)
            duration: 400
            easing.type: Easing.InExpo
        }
        PropertyAnimation {
            id: animationBack2
            property: "opacity"
            target: root
            from: 1
            to: 0
            duration: 300
        }
    }
    DropShadow {
        anchors.fill: base
        horizontalOffset: 0
        verticalOffset: Fit.fit(3)
        radius: (8.0)
        samples: (18)
        color: "#10000000"
        source: base
    }
    Item {
        id: container
        anchors.fill: base
        anchors.topMargin: base.radius+Fit.fit(5)
    }
    onContentItemChanged: {
        contentItem.parent = container
        contentItem.anchors.fill = container
    }
    function show() {
        if (animationForth.running) return
        animationForth.restart()
        animationForth2.restart()
        root.visible = true
    }
    function hide() {
        if (animationBack.running) return
        animationBack.restart()
        DelayCaller.delayCall(200, animationBack2.restart)
        DelayCaller.delayCall(500, function() { root.visible = false; })
    }
    property alias base: base
    property var contentItem
}
