import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.0
import "fit.js" as Fit

Popup {
    id: root
    enter: Transition {
        NumberAnimation {
            property: "scale"
            duration: 500
            easing.type: Easing.OutElastic
            from: 0
            to: 1
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "scale"
            duration: 200
            easing.type: Easing.InBack
            to: 0
            from: 1
        }
    }

    background: Rectangle {
        color: "transparent"
        Rectangle {
            id: popup
            anchors.fill: parent
            color: "transparent"
            Canvas {
                id: triangle
                antialiasing: true
                anchors { top: parent.top;}
                height: parent.height / 6.0
                width: height + height/2.0

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.save();
                    ctx.clearRect(0,0,triangle.width, triangle.height);
                    ctx.lineJoin = "round";
                    ctx.fillStyle = root.color
                    ctx.beginPath();
                    // put rectangle in the middle
                    ctx.translate((0.5 *width - 0.5*width),
                                  (0.5 * height - 0.5 * height))
                    // draw the rectangle
                    ctx.moveTo(0,height ); // left point of triangle
                    ctx.lineTo(width/2.0, 0);
                    ctx.lineTo(width,height);
                    ctx.closePath();
                    ctx.fill();
                    ctx.restore();
                }
            }
            Rectangle {
                clip: true
                radius: root.radius
                anchors { left: parent.left; right: parent.right;
                    bottom: parent.bottom; top: triangle.bottom;}
                gradient: Gradient {
                    GradientStop { position: 0.0; color: root.color }
                    GradientStop { position: 1.0; color: Qt.darker(root.color, 1.08) }
                }
            }
        }

        DropShadow {
            anchors.fill: popup
            horizontalOffset: Fit.fit(3)
            verticalOffset: Fit.fit(3)
            radius: Fit.fit(8)
            samples: 28
            color: "#70000000"
            source: popup
        }
    }

    property color color: "#e6e6e6"
    property int radius: Fit.fit(5)
    property int xCord: 0
    property int yCord: 0
    function show(xCoord, yCoord) {
        root.xCord = xCoord
        root.yCord = yCoord
        root.y = yCoord
        triangle.x = ((width - triangle.width)/1.2 - triangle.width/2.0)
        var X = xCoord - ((width - triangle.width)/1.2 - triangle.width/4.0)
        root.x = X
        if (X <= Fit.fit(10)){
            root.x = Fit.fit(10)
            triangle.x = triangle.x + X - Fit.fit(10)
        }
        root.open()
        root.forceActiveFocus()
    }
}
