import QtQuick 2.7
import QtQuick.Controls 1.4

ApplicationWindow {
    id: applicationWindow
    width: 640
    height: 480
    visible: true
    color: "red"
    title: qsTr("Hello World")

    menuBar: MenuBar {
        Menu {
            title: qsTr("Menu")
            MenuItem {
                text: qsTr("&Option 1")
                onTriggered: console.log("Option 1 triggered");
            }
            MenuItem {
                text: qsTr("&Option 2")
                onTriggered: console.log("Option 2 triggered");
            }
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
    }
}
