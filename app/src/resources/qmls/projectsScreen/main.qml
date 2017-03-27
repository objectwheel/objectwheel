import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

// mark active/open project in the list

Rectangle {
    color: "#E0E4E7"
    SwipeView {
        anchors.fill: parent
        ProjectsPage {
            id: projectsPage
            logoPath: "qrc:///resources/images/logo.png"
            projectList.listView.model: ListModel {
                ListElement {
                    projectName: "Bill Smith"
                    lastEdited: "555 3264"
                }
                ListElement {
                    projectName: "John Brown"
                    lastEdited: "555 8426"
                }
                ListElement {
                    projectName: "Sam Wise"
                    lastEdited: "555 0473"
                }
            }
        }

        ProjectSettings {
            id: projectSettings
        }

    }
    function fit(val) { return val }
}
