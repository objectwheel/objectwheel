import QtQuick 2.12
import QtMultimedia 5.12

Item {
    property var source
    property bool autoOrientation : false
    property int orientation : 0
    property int fillMode : VideoOutput.PreserveAspectFit
    property int flushMode : VideoOutput.EmptyFrame
    signal sourceRectChanged()
    signal contentRectChanged()
}
