import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    color: "#E0E4E7"
    SwipeView {
        id: swipeView
        anchors.fill: parent
        interactive: false
        clip: true
        ProjectsPage {
            id: projectsPage
            clip: true
            logoPath: "qrc:///resources/images/logo.png"
            version: owVersion
        }

        ProjectSettings {
            id: projectSettings
            clip: true
        }
    }

    property alias swipeView: swipeView
    property alias projectSettings: projectSettings
    property alias projectsPage: projectsPage
    property string owVersion
}
