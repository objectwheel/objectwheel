import QtQuick 2.7
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.1
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller

Rectangle {
    id: root
    color: "#E0E4E7"

    SwipeView {
        id: swipeView
        anchors.fill: parent
        interactive: false
        clip: true
        currentIndex:  0

        Item {
            LoginScreen {
                id: loginScreen
                popup: popup
                y: parent.height/2.0 - height/2.0 - Fit.fit(15)
                x: parent.width/2.0 - width/2.0
                logoPath: "qrc:///resources/images/logo.png"
                loginButton.onSignupButtonClicked: swipeView.currentIndex = 1
            }
            clip: true
        }
        RegistrationScreen {
            Component.onCompleted: {
                btnCancel.clicked.connect(function() {
                    swipeView.currentIndex = 0
                })
                btnOk.clicked.connect(function() {
                    swipeView.currentIndex = 2
                    regCompleteScreen.animate()
                })
            }
            clip: true
        }
        RegistrationComplete {
            id: regCompleteScreen
            Component.onCompleted: {
                btnOkay.clicked.connect(function() {
                    swipeView.currentIndex = 0
                })
            }
            clip: true
        }
    }

    Text {
        text: "© 2015 - 2018 Objectwheel, Inc. All Rights Reserved."
        x: parent.width/2.0 - width/2.0
        anchors.bottom: parent.bottom
        font.weight: Font.Bold
        color: "#2E3A41"
        anchors.bottomMargin:  Fit.fit(2)
        opacity: swipeView.currentIndex == 0
        Behavior on opacity {
            PropertyAnimation {}
        }
    }
    Toast {
        id: toast
        anchors.fill: parent
    }
    PopupScreen {
        id: popup
        anchors.fill: parent
        base.width: Fit.fit(280)
        base.height: Fit.fit(114)
        base.radius: Fit.fit(8)
        base.color: "#D0D4D7"
        base.border.color: "#a0a4a7"
    }
    property alias toast: toast
    property alias loginScreen: loginScreen
}
