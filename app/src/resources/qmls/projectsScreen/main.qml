import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

// mark active/open project in the list

Window {
    visible: true
    width: 700
    height: 430
    title: qsTr("Objectwheel")
    color: "#E0E4E7"

    SwipeView {
        anchors.fill: parent
        ProjectsPage {
            id: projectsPage
            logoPath: "images/logo.png"
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
//    Timer {
//        running: true
//        interval: 5000
//        onTriggered: {
//            console.log(projectsPage.projectList.selectedProjectname);
//            projectsPage.projectList.listView.model.append({"projectName": "Ömer Göktaş", "lastEdited":"11.04.2017"})
//        }
//    }

    function fit(val) { return val }
}
