import QtQuick 2.7
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "../fit.js" as Fit

Column {
    spacing: Fit.fit(12)
    Image {
        id: welcomeImage
        anchors.horizontalCenter: parent.horizontalCenter
        source: logoPath
        height: Fit.fit(80)
        fillMode: Image.PreserveAspectFit
    }
    Image {
        id: loginImage
        anchors.horizontalCenter: parent.horizontalCenter
        source: "qrc:///resources/images/login.png"
        height: Fit.fit(18)
        fillMode: Image.PreserveAspectFit
    }
    Item {
        id: inputContainer
        width: loginButton.width
        height: Fit.fit(70)
        Rectangle {
            id: base
            anchors.fill: parent
            color: "#d0d4d7"
            border.color: "#c2c6c9"
            radius: Fit.fit(8)
            border.width: Fit.fit(1)
            visible: false
        }
        DropShadow {
            anchors.fill: base
            horizontalOffset: 0
            verticalOffset: Fit.fit(1)
            radius: Fit.fit(7.0)
            samples: Fit.fit(25)
            color: "#20000000"
            source: base
        }
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            RowLayout {
                Text {
                    id: emailLabel
                    text: "Email"
                    font.pixelSize: Fit.fit(13)
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillHeight: true
                    color: "#45000000"
                    Layout.leftMargin: Fit.fit(10)
                    Rectangle {
                        id: emailWrongRect
                        anchors.verticalCenter: parent.verticalCenter
                        height: Fit.fit(8)
                        width: height
                        color: !validateEmail(emailTextInput.text) ? "#db2312" : "#8bbb56"
                        border.color: !validateEmail(emailTextInput.text) ? "#9b2312" : "#6d9f45"
                        radius: width / 2
                        anchors.right: parent.left
                        anchors.rightMargin: Fit.fit(20)
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: emailTextInput.focus = true
                    }
                }
                TextField {
                    id: emailTextInput
                    clip: true
                    validator: RegExpValidator { regExp: /^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,4}$/ }
                    selectByMouse: true
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhEmailCharactersOnly
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    verticalAlignment: TextArea.AlignVCenter
                    horizontalAlignment: TextArea.AlignRight
                    font.pixelSize: Fit.fit(13)
                    Layout.rightMargin: Fit.fit(10)
                    style: TextFieldStyle {
                        textColor: "#e0000000"
                        background: Rectangle {
                            radius: 2
                            color: "transparent"
                        }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                height: Fit.fit(1)
                color: "#c2c6c9"
                Layout.rightMargin: Fit.fit(2)
                Layout.leftMargin: Fit.fit(2)
            }
            RowLayout {
                Text {
                    id: passwordLabel
                    text: "Password"
                    font.pixelSize: Fit.fit(13)
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillHeight: true
                    color: "#45000000"
                    Layout.leftMargin: Fit.fit(10)
                    Rectangle {
                        id: passWrongRect
                        anchors.verticalCenter: parent.verticalCenter
                        height: Fit.fit(8)
                        width: height
                        color: !validatePassword(passwordTextInput.text) ? "#db2312" : "#8bbb56"
                        border.color: !validatePassword(passwordTextInput.text) ? "#9b2312" : "#6d9f45"
                        radius: width / 2
                        anchors.right: parent.left
                        anchors.rightMargin: Fit.fit(20)
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: passwordTextInput.focus = true
                    }
                }
                TextField {
                    id: passwordTextInput
                    validator: RegExpValidator { regExp: /^[><{}\[\]*!@\-#$%^&+=~\.\,\:a-zA-Z0-9]{6,25}$/ }
                    selectByMouse: true
                    echoMode: showPassButton.checked ? TextInput.Normal : TextInput.Password
                    inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    verticalAlignment: TextArea.AlignVCenter
                    horizontalAlignment: TextArea.AlignRight
                    font.pixelSize: Fit.fit(13)
                    Layout.rightMargin: Fit.fit(10)
                    style: TextFieldStyle {
                        textColor: "#e0000000"
                        background: Rectangle {
                            radius: 2
                            color: "transparent"
                        }
                    }
                }
            }
        }
        FancyButton {
            id: showPassButton
            iconButton: false
            height: width
            base.radius: Fit.fit(3)
            base.color: !checked ? "transparent" : "#f0f4f7"
            base.border.color: !checked ? "transparent" : "#e0e4e7"
            checkable: true
            width: Fit.fit(15)
            anchors.left: base.right
            anchors.bottom: base.bottom
            anchors.leftMargin: Fit.fit(8)
            anchors.bottomMargin: Fit.fit(10)
            Image {
                anchors.fill: parent
                anchors.margins: Fit.fit(1)
                source: "qrc:///resources/images/find.png"
                fillMode: Image.PreserveAspectFit
            }
        }
    }
    Item {
        width: loginButton.width
        height: Fit.fit(35)
        Rectangle {
            id: base2
            anchors.fill: parent
            radius: Fit.fit(20)
            color: "#d0d4d7"
            border.color: "#c2c6c9"
        }
        DropShadow {
            anchors.fill: base2
            horizontalOffset: 0
            verticalOffset: Fit.fit(1)
            radius: Fit.fit(7.0)
            samples: Fit.fit(25)
            color: "#20000000"
            source: base2
        }
        RowLayout {
            spacing: Fit.fit(8)
            width: parent.width - Fit.fit(10)
            anchors.centerIn: parent
            Switch {
                id: autologinSwitch
                checked: false
                width: Fit.fit(42)
                height: Fit.fit(25)
                style: SwitchStyle {
                    groove: Rectangle {
                        implicitWidth: control.width
                        implicitHeight: control.height
                        radius: implicitHeight / 2.0
                        color: control.checked ? "#86B753" : "#c0c4c7"
                        border.color: control.checked ? "#86B753" : "#a0a4a7"
                        border.width: Fit.fit(1)
                    }

                    handle: Item {
                        implicitWidth: control.height
                        implicitHeight: control.height
                        Rectangle {
                            id: handle
                            implicitWidth: control.height
                            implicitHeight: control.height

                            radius: parent.implicitHeight / 2.0
                            color: "#c0c4c7"
                            border.color: "#a0a4a7"
                            border.width: Fit.fit(0.75)
                            visible: false
                        }
                        DropShadow {
                            anchors.fill: handle
                            horizontalOffset: 0
                            verticalOffset: Fit.fit(2.0)
                            radius: Fit.fit(8)
                            samples: Fit.fit(15)
                            color: "#35000000"
                            source: handle
                        }
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                }
            }
            Text {
                text: "Automatic login"
                font.pixelSize: Fit.fit(13)
                verticalAlignment: Text.AlignVCenter
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "#4E5051"
            }
        }
    }
    PropertyAnimation {
        id: wrongPassAnim
        target: passwordTextInput
        property: "x"
        to: passwordTextInput.x
        from: passwordTextInput.x - Fit.fit(12)
        easing.type: Easing.OutElastic
        duration: 500
    }
    PropertyAnimation {
        id: wrongEmailAnim
        target: emailTextInput
        property: "x"
        to: emailTextInput.x
        from: emailTextInput.x - Fit.fit(12)
        easing.type: Easing.OutElastic
        duration: 500
    }
    ColorAnimation {
        id: colorAnim
        to: "#45000000"
        from: "#aaa91010"
        duration: 4000
        property: "color"
        easing.type: Easing.InExpo
    }
    LoginButton {
        id: loginButton
        height: Fit.fit(28)
        width: Fit.fit(300)
        loginButtonDisabled: !validateEmail(emailTextInput.text) || !validatePassword(passwordTextInput.text)
        onLoginButtonClicked: animateWrongPass()
    }
    FancyButton {
        id: infoButton
        iconButton: true
        iconSource: "qrc:///resources/images/info.png"
        anchors.horizontalCenter: parent.horizontalCenter
        height: width
        width: Fit.fit(25)
        onClicked: popup.show()
    }
    Component.onCompleted: popup.contentItem = ctx.createObject(null)
    Component {
        id: ctx
        Item {
            anchors.fill: parent
            Item {
                id: padder
                anchors.top: parent.top
                width: 1
                height: Fit.fit(6)
            }
            Image {
                id: img
                anchors.top: padder.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:///resources/images/info.png"
                width: Fit.fit(20)
                fillMode: Image.PreserveAspectFit
            }
            FancyButton {
                id: lostPassButton
                text.text: "Lost your password?"
                text.color: "#4E5051"
                text.font.pixelSize: Fit.fit(13)
                width: parent.width
                height: (parent.height - img.height - padder.height) / 2
                anchors.top: img.bottom
                iconButton: false
                base.color: pressing ? "#20000000" : "transparent"
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    x: Fit.fit(35)
                    source: "qrc:///resources/images/key.png"
                    height: Fit.fit(16)
                    fillMode: Image.PreserveAspectFit
                }
            }
            Rectangle {
                id: line
                anchors { top: lostPassButton.bottom; left: parent.left; right: parent.right; leftMargin: Fit.fit(2); rightMargin: Fit.fit(2); }
                width: parent.width - Fit.fit(4)
                height: Fit.fit(1)
                color: "#15000000"
            }
            FancyButton {
                id: aboutButton
                text.text: "About Objectwheel"
                text.color: "#4E5051"
                text.font.pixelSize: Fit.fit(13)
                width: parent.width
                height: (parent.height - img.height - padder.height) / 2
                anchors.top: line.bottom
                iconButton: false
                base.color: "transparent"
                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    x: Fit.fit(40)
                    source: "qrc:///resources/images/aboutus.png"
                    height: Fit.fit(16)
                    fillMode: Image.PreserveAspectFit
                }
            }
            Rectangle {
                id: shadow
                color: "#30000000"
                anchors.fill: aboutButton
                visible: false
            }
            Rectangle {
                id: mask
                anchors.fill: aboutButton
                anchors.bottomMargin: Fit.fit(2)
                radius: Fit.fit(8)
                Rectangle {
                    width: parent.width
                    height: Fit.fit(10)
                }
                visible: false
            }
            OpacityMask {
                anchors.fill: shadow
                anchors.bottomMargin: Fit.fit(2)
                source: shadow
                maskSource: mask
                visible: aboutButton.pressing
            }
        }
    }
    function validatePassword(pw) {
        var re = /^[><{}\[\]*!@\-#$%^&+=~\.\,\:a-zA-Z0-9]{6,25}$/
        return re.test(pw);
    }
    function validateEmail(email) {
        var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/
        return re.test(email);
    }
    function animateWrongPass() {
        if (wrongPassAnim.running) return
        wrongPassAnim.restart()
        colorAnim.target = passwordLabel
        colorAnim.restart()
    }
    function animateWrongEmail() {
        if (wrongEmailAnim.running) return
        wrongEmailAnim.restart()
        colorAnim.target = emailLabel
        colorAnim.restart()
    }
    property alias loginButton : loginButton
    property var popup
    property string logoPath
}
