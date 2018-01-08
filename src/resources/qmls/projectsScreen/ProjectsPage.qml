import QtQuick 2.7
import QtGraphicalEffects 1.0
import "../fit.js" as Fit

Item {
    Image {
        id: welcomeImage
        source: logoPath
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: Fit.fit(15)
        height: Fit.fit(80)
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: welcomeTextImage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcomeImage.bottom
        anchors.topMargin: Fit.fit(10)
        text: "Welcome to Objectwheel"
        color: "#1E2A31"
        font.weight: Font.ExtraLight
        font.pixelSize: Fit.fit(28)
    }

    Text {
        id: versionTextImage
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcomeTextImage.bottom
        anchors.topMargin: Fit.fit(15)
        color: "#2E3A41"
        text: "Version " + version
        font.pixelSize: Fit.fit(15)
        font.weight: Font.Light
    }

    Text {
        id: infText
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: versionTextImage.bottom
        anchors.topMargin: Fit.fit(15)
        text: "Your Projects"
        color: "#2E3A41"
    }

    ProjectList {
        id: projectList
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: infText.bottom
        anchors.topMargin: Fit.fit(1)
        anchors.bottom: projectButton.top
        anchors.bottomMargin: Fit.fit(15)
        width: parent.width - Fit.fit(50)
    }

    ProjectButton {
        id: projectButton
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Fit.fit(15)
        height: Fit.fit(28) - (Fit.fit(28) % 1)
        width: Fit.fit(240) - (Fit.fit(240) % 2)
        loadButtonDisabled: projectList.listView.count < 1
    }
    property alias projectList : projectList
    property alias projectButton : projectButton
    property string logoPath
    property string version
}
