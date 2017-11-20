var timerString = "import QtQuick 2.0 \n Timer {}"
function delayCall(interval, callback) {
    var delayCaller = Qt.createQmlObject(timerString, root)
    delayCaller.interval = interval
    delayCaller.triggered.connect( function () {
        callback();
        delayCaller.destroy();
    });
    delayCaller.start();
}
