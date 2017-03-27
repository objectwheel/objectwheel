import QtQuick 2.7
import QtGraphicalEffects 1.0

Item {
    Image {
        id: welcomeImage
        source: logoPath
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: fit(15)
        height: fit(80)
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: welcomeTextImage
        source: "images/welcome.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcomeImage.bottom
        anchors.topMargin: fit(10)
        fillMode: Image.PreserveAspectFit
        height: fit(28)
    }

    Image {
        id: versionTextImage
        source: "images/version.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcomeTextImage.bottom
        anchors.topMargin: fit(15)
        fillMode: Image.PreserveAspectFit
        height: fit(12)
    }

    Text {
        id: infText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: versionTextImage.bottom
        anchors.topMargin: fit(15)
        text: "Your Projects"
        color: "#505457"
        font.pixelSize: fit(13)
    }

    ProjectList {
        id: projectList
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: infText.bottom
        anchors.topMargin: fit(1)
        anchors.bottom: projectButton.top
        anchors.bottomMargin: fit(15)
        width: parent.width - fit(50)
    }

    ProjectButton {
        id: projectButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: fit(15)
        height: fit(28)
        width: fit(240)
        loadButtonDisabled: projectList.listView.count < 1
    }
    property alias projectList : projectList
    property alias projectButton : projectButton
    property string logoPath
    function fit(val) { return val }
}
