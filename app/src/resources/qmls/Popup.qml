import QtQuick 2.7
import QtQuick.Layouts 1.1
import "delaycaller.js" as DelayCaller
import "fit.js" as Fit

Rectangle {
    id: root
    visible: false
    MouseArea { anchors.fill: parent;  onClicked: close() }
    ColumnLayout {
        id :menu
        spacing: d.spacing
        anchors.margins: d.spacing
        anchors { left: root.left; right: root.right }
        Rectangle {
            id: container
            color: containerColor
            radius: d.radius
            Layout.fillWidth: true
            MouseArea { anchors.fill: parent; }
        }
        FancyButton {
            id: cancelButton
            base.color: containerColor
            base.radius: d.radius
            text.text: "Cancel"
            text.font.bold: true
            text.color: "#0061ea"
            notIconButton: true
            Layout.preferredHeight: d.cancelButtonHeight
            Layout.fillWidth: true
            onClicked: close()
        }
    }
    ColorAnimation {
        id: colorAnimation
        target: root
        duration: d.duration
        property: "color"
        onStopped: if (root.state === "close") { root.visible = false; root.color = d.colorBuff }
    }
    QtObject {
        id: d
        function showBackground() {
            d.colorBuff = root.color
            colorAnimation.to = root.color
            colorAnimation.from = "transparent"
            colorAnimation.start()
            root.visible = true
        }
        function hideBackground() {
            colorAnimation.from = root.color
            colorAnimation.to = "transparent"
            colorAnimation.start()
        }
        property Item itemBuff: null
        property color colorBuff
        property real cancelButtonHeight: Fit.fit(35)
        property real radius: Fit.fit(8)
        property real spacing: Fit.fit(5)
        property int duration: 500
    }
    states: [
        State {
            name: "open"
            AnchorChanges { target: menu; anchors { top: undefined; bottom: root.bottom } }
        },
        State {
            name: "close"
            AnchorChanges { target: menu; anchors { top: root.bottom; bottom: undefined } }
        }
    ]
    transitions: Transition {
        AnchorAnimation { duration: d.duration; easing.type: Easing.OutExpo }
    }
    Component.onCompleted: root.state = "close"
    onContainerItemChanged: {
        if (d.itemBuff) d.itemBuff.visible = false
        d.itemBuff = containerItem
        container.Layout.preferredHeight = containerItem.height
        containerItem.parent = container
        containerItem.anchors.centerIn = container
        containerItem.width = Qt.binding(function(){ return container.width })
        containerItem.visible = true
    }
    function open() {
        if (state === "close") {
            d.showBackground()
            root.state = "open"
        }
    }
    function close() {
        if (state === "open") {
            d.hideBackground()
            root.state = "close"
        }
    }
    readonly property bool opened: state === "open"
    property Item containerItem: null
    property color containerColor: "white"
    property alias cancelButton: cancelButton
}
