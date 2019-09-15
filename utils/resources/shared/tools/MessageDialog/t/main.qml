import QtQuick 2.12
import QtQuick.Dialogs 1.3

MessageDialog {
    id: messageDialog
    text: qsTr("Text")
    title: qsTr("Title")
    informativeText: qsTr("Informative Text")
    icon: MessageDialog.Information
    standardButtons : MessageDialog.Ok | MessageDialog.Cancel
}
