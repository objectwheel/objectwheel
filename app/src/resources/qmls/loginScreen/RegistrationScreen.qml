import QtQuick 2.7
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.0
import "../fit.js" as Fit
import "../delaycaller.js" as DelayCaller

Item {
    id: root
    Rectangle {
        id: title
        color: "#25000000"
        anchors { top: parent.top; left: parent.left; right: parent.right;}
        height: Fit.fit(40)
        Image {
            id: projectsettingsTextImage
            source: "qrc:///resources/images/registration.png"
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
            fillMode: Image.PreserveAspectFit
            height: Fit.fit(16)
        }
        Item {
            id: btnCancelOutCont
            anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: Fit.fit(10) }
            width: Fit.fit(80)
            height: Fit.fit(28)
            Item {
                id: btnCancelInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnCancel
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#EA8622" }
                        GradientStop { position: 1.0; color: "#DC521A" }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Text {
                            text: "Cancel"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnCancelImg.height
                            font.pixelSize: Fit.fit(13)
                        }
                        Image {
                            id: btnCancelImg
                            source: "qrc:///resources/images/cancel.png"
                            height: btnCancel.height - Fit.fit(14)
                            fillMode: Image.PreserveAspectFit
                        }
                    }
                    signal clicked();
                    function glow() {
                        btnCancelOverlay.color = "#30000000"
                    }
                    function unglow() {
                        btnCancelOverlay.color = "#00ffffff"
                    }
                }
                ColorOverlay {
                    id: btnCancelOverlay
                    anchors.fill: btnCancel
                    source: btnCancel
                    color: "#00ffffff"
                }
            }
            Rectangle {
                id: btnCancelMask;
                anchors.fill: parent;
                radius: Fit.fit(6);
                Rectangle {
                    height: parent.height
                    width: parent.width / 2
                    anchors.right: parent.right
                    radius: Fit.fit(2)
                }
                visible: false;
            }
            OpacityMask {
                id: btnCancelOpMask
                visible: false
                anchors.fill: btnCancelInCont
                source: btnCancelInCont
                maskSource: btnCancelMask
            }
            DropShadow {
                anchors.fill: btnCancelOpMask
                horizontalOffset: 0
                verticalOffset: Fit.fit(1)
                radius: 3
                samples: 15
                color: "#30000000"
                source: btnCancelOpMask
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: btnCancel.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onPressed: btnCancel.glow()
                onReleased: btnCancel.unglow()
                onClicked: btnCancel.clicked()
            }
        }
        Item {
            id: btnOkOutCont
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: Fit.fit(10) }
            width: Fit.fit(80)
            height: Fit.fit(28)
            Item {
                id: btnOkInCont
                anchors.fill: parent
                visible: false
                Rectangle {
                    id: btnOk
                    anchors.fill: parent
                    visible: false
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#8bbb56" }
                        GradientStop { position: 1.0; color: "#6d9f45" }
                    }
                    Row {
                        anchors.centerIn: parent
                        spacing: Fit.fit(6)
                        Image {
                            id: btnOkImg
                            source: "qrc:///resources/images/ok.png"
                            height: btnOk.height - Fit.fit(14)
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            text: "Ok"
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            height: btnOkImg.height
                            font.pixelSize: Fit.fit(13)
                        }
                    }
                    signal clicked();
                    function glow() {
                        btnOkOverlay.color = "#30000000"
                    }
                    function unglow() {
                        btnOkOverlay.color = "#00ffffff"
                    }
                }
                ColorOverlay {
                    id: btnOkOverlay
                    anchors.fill: btnOk
                    source: btnOk
                    color: "#00ffffff"
                }
            }
            Rectangle {
                id: btnOkMask;
                anchors.fill: parent;
                radius: Fit.fit(6);
                Rectangle {
                    height: parent.height
                    width: parent.width / 2
                    anchors.left: parent.left
                    radius: Fit.fit(2)
                }
                visible: false;
            }
            OpacityMask {
                id: btnOkOpMask
                visible: false
                anchors.fill: btnOkInCont
                source: btnOkInCont
                maskSource: btnOkMask
            }
            DropShadow {
                anchors.fill: btnOkOpMask
                horizontalOffset: 0
                verticalOffset: Fit.fit(1)
                radius: 3
                samples: 15
                color: "#30000000"
                source: btnOkOpMask
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: btnOk.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onPressed: btnOk.glow()
                onReleased: btnOk.unglow()
                onClicked: {
                    if (firstnameTextInput.text === '' ||
                            lastnameTextInput.text === '' ||
                            emailTextInput.text === '' ||
                            passwordTextInput.text === '') {
                        warningText.text = "Please fill all required fields."
                        warning.show()
                    } else if (passwordTextInput.text !== passwordTextInput2.text) {
                        warningText.text = "Passwords do not match."
                        warning.show()
                    } else if (emailTextInput.text !== emailTextInput2.text) {
                        warningText.text = "Emails do not match."
                        warning.show()
                    } else if (!validateEmail(emailTextInput.text)) {
                        warningText.text = "Please make sure your email\naddress is correct."
                        warning.show()
                    } else if (!validatePassword(passwordTextInput.text)) {
                        warningText.text = "Please make sure your password's length\nis in between 6 and 20."
                        warning.show()
                    } else { //FIXME: http -> https | change ip with domain name
                        var url = "http://165.227.164.200/api/v1/registration/register";
                        var body = {
                            first: firstnameTextInput.text,
                            last: lastnameTextInput.text,
                            email: emailTextInput.text,
                            pw: passwordTextInput.text,
                            country: countryTextInput.text,
                            company: companyTextInput.text,
                            title: titleTextInput.text,
                            phone: phoneTextInput.text
                        }
                        var http = new XMLHttpRequest();
                        http.open("POST", url, true);
                        http.setRequestHeader("content-type", "application/json");
                        http.onreadystatechange = function() {
                            if(http.readyState == 4 && http.status == 200) {
                                var data = JSON.parse(http.responseText);
                                if (data.result === 'OK') {
                                    firstnameTextInput.text = ""
                                    lastnameTextInput.text = ""
                                    emailTextInput.text = ""
                                    emailTextInput2.text = ""
                                    passwordTextInput.text = ""
                                    passwordTextInput2.text = ""
                                    countryTextInput.text = ""
                                    companyTextInput.text = ""
                                    titleTextInput.text = ""
                                    phoneTextInput.text = ""
                                    btnOk.clicked()
                                } else {
                                    warningText.text = "Given email address has already been\nused. Please check your mailbox\nfor verification, or try to login."
                                    warning.show()
                                }
                            } else if (http.status != 200) {
                                warningText.text = "Please check your\ninternet connection"
                                warning.show()
                            }
                        }
                        http.send(JSON.stringify(body));
                    }
                }
            }
        }
        Rectangle {
            id: titleLine
            anchors.bottom: parent.bottom
            width: parent.width
            height: Fit.fit(1)
            color: "#25000000"
        }
    }
    Flickable {
        id: flickable
        clip: true
        anchors { top: title.bottom; bottom: parent.bottom; right: parent.right; left:parent.left; }
        ScrollBar.vertical: ScrollBar { }
        contentWidth: width
        contentHeight: firstnameContainer.height + lastnameContainer.height +
                       emailContainer.height + passwordContainer.height +
                       emailContainer2.height + passwordContainer2.height +
                       countryContainer.height + companyContainer.height +
                       titleContainer.height + phoneContainer.height
        Column {
            anchors.fill: parent
            Rectangle {
                id: firstnameContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: firstnameTitle
                        text: "First Name*"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: firstnameTextInput
                        clip: true
                        textColor: "#2E3A41"
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: lastnameContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: lastnameTitle
                        text: "Last Name*"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: lastnameTextInput
                        clip: true
                        textColor: "#2E3A41"
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: emailContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: emailTitle
                        text: "Email*"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextField {
                        id: emailTextInput
                        clip: true
                        textColor: "#2E3A41"
                        validator: RegExpValidator { regExp: /^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,4}$/ }
                        selectByMouse: true
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhEmailCharactersOnly
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                        style: TextFieldStyle {
                            background: Rectangle {
                                radius: 2
                                color: "transparent"
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: emailContainer2
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: emailTitle2
                        text: "Confirm Email*"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextField {
                        id: emailTextInput2
                        clip: true
                        textColor: "#2E3A41"
                        validator: RegExpValidator { regExp: /^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,4}$/ }
                        selectByMouse: true
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhEmailCharactersOnly
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                        style: TextFieldStyle {
                            background: Rectangle {
                                radius: 2
                                color: "transparent"
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: passwordContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: passwordTitle
                        text: "Password*"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextField {
                        id: passwordTextInput
                        textColor: "#2E3A41"
                        validator: RegExpValidator { regExp: /^[><{}\[\]*!@\-#$%^&+=~\.\,\:a-zA-Z0-9]{6,25}$/ }
                        selectByMouse: true
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                        style: TextFieldStyle {
                            background: Rectangle {
                                radius: 2
                                color: "transparent"
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: passwordContainer2
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: passwordTitle2
                        text: "Confirm Password*"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextField {
                        id: passwordTextInput2
                        textColor: "#2E3A41"
                        validator: RegExpValidator { regExp: /^[><{}\[\]*!@\-#$%^&+=~\.\,\:a-zA-Z0-9]{6,25}$/ }
                        selectByMouse: true
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                        style: TextFieldStyle {
                            background: Rectangle {
                                radius: 2
                                color: "transparent"
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: countryContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: countryTitle
                        text: "Country"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: countryTextInput
                        clip: true
                        textColor: "#2E3A41"
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: companyContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: companyTitle
                        text: "Company"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: companyTextInput
                        clip: true
                        textColor: "#2E3A41"
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: titleContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "#07000000"
                border.color: "#14000000"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: titleTitle
                        text: "Title"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: titleTextInput
                        clip: true
                        textColor: "#2E3A41"
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
            Rectangle {
                id: phoneContainer
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: Fit.fit(40)
                color: "transparent"
                RowLayout {
                    anchors.fill: parent
                    anchors { leftMargin: Fit.fit(10); rightMargin: Fit.fit(10); }
                    Text {
                        id: phoneTitle
                        text: "Phone"
                        color: "#2E3A41"
                        font.bold: true
                        font.pixelSize: Fit.fit(13)
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillHeight: true
                    }
                    TextArea {
                        id: phoneTextInput
                        clip: true
                        textColor: "#2E3A41"
                        selectByKeyboard: true
                        selectByMouse: true
                        frameVisible: false
                        backgroundVisible: false
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        verticalAlignment: TextArea.AlignVCenter
                        horizontalAlignment: TextArea.AlignRight
                        font.pixelSize: Fit.fit(13)
                    }
                }
            }
        }
    }
    Item {
        id: warning
        y: -height - Fit.fit(10)
        width: parent.width / 2.4
        height: Fit.fit(65)
        anchors.horizontalCenter: parent.horizontalCenter
        Rectangle {
            id: base
            anchors.fill: parent
            radius: Fit.fit(7)
            visible: false
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter("#C0C3C5",1.03) }
                GradientStop { position: 1.0; color: Qt.darker("#C0C3C5",1.03) }
            }
            border.color: "#B0AFB0"
        }
        DropShadow {
            anchors.fill: parent
            horizontalOffset: 0
            verticalOffset: Fit.fit(1)
            radius: (2.0)
            samples: (17)
            color: "#40000000"
            source: base
        }
        Text {
            id: warningText
            font.pixelSize: Fit.fit(13)
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            y: parent.height/2.0 -contentHeight/2.0 + Fit.fit(4)
            color: "#2E3A41"

            Text {
                font.pixelSize: Fit.fit(16)
                font.bold: true
                text: "*"
                anchors.verticalCenter: parent.verticalCenter
                x: parent.width/2.0 - parent.contentWidth/2.0 - Fit.fit(10)
                color: "#bb3333"
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: 350
                easing.type: Easing.InExpo
            }
        }
        function show() {
            y = -Fit.fit(10)
            DelayCaller.delayCall(5000, function() {
                y = -height - Fit.fit(10)
            })
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
    property alias btnOk: btnOk
    property alias btnCancel: btnCancel
}
