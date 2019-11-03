import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
    id: dialog
    visible: true
    title: qsTr("Dialog")
    standardButtons: Dialog.Ok | Dialog.Cancel
}
