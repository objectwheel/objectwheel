import QtQuick 2.7
import QtQuick.Layouts 1.1
import "fit.js" as Fit

Rectangle {
    RowLayout {
        spacing: 0
        anchors { fill: parent; margins: Fit.fit(4) }
        FancyButton {
            id: backButton
            iconSource: backIcon
            Layout.preferredWidth: height
            Layout.fillHeight: true
            onClicked: backButtonClicked()
        }
        FancyButton {
            id: homeButton
            iconSource: homeIcon
            Layout.preferredWidth: height
            Layout.fillHeight: true
            onClicked: homeButtonClicked()
        }
        FancyButton {
            id: newFolderButton
            iconSource: newFolderIcon
            Layout.preferredWidth: height
            Layout.fillHeight: true
            onClicked:newFolderButtonClicked()
        }
        FancyButton {
            id: newFileButton
            iconSource: newFileIcon
            Layout.preferredWidth: height
            Layout.fillHeight: true
            onClicked: newFileButtonClicked()
        }
        FancyButton {
            id: downloadButton
            iconSource: downloadIcon
            Layout.preferredWidth: height
            Layout.fillHeight: true
            onClicked: downloadButtonClicked()
        }
        Item { Layout.fillWidth: true }
    }
    signal backButtonClicked()
    signal homeButtonClicked()
    signal newFolderButtonClicked()
    signal newFileButtonClicked()
    signal downloadButtonClicked()
    property string backIcon
    property string homeIcon
    property string newFolderIcon
    property string newFileIcon
    property string downloadIcon
}
