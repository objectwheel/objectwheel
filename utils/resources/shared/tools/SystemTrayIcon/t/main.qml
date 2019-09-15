import QtQuick 2.12
import Qt.labs.platform 1.1

SystemTrayIcon {
    id: systemTrayIcon
    icon.source: "qrc:/qt-project.org/styles/commonstyle/images/up-128"
    menu: Menu {
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
    }
}
