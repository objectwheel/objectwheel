TEMPLATE = app
TARGET = Objectwheel

QT += widgets quickwidgets webenginewidgets concurrent
QT += network qml quick quickcontrols2 webview websockets webengine webchannel webenginecore
QT += sensors svg scxml purchasing positioning nfc multimedia location gamepad datavisualization
QT += charts 3dcore 3drender 3dinput 3dlogic 3dextras 3danimation 3dquick bluetooth

include($$PWD/gitversion.pri)
include($$PWD/src/src.pri)
include($$PWD/contrib/contrib.pri)

DEFINES += QT_QML_DEBUG_NO_WARNING \
           APP_VER='"\\\"$$VERSION\\\""' \
           APP_NAME='"\\\"$$TARGET\\\""' \
           APP_CORP='"\\\"Objectwheel, Inc.\\\""' \
           APP_DOMAIN='"\\\"objectwheel.com\\\""' \
           APP_WSSSERVER='"\\\"wss://api.objectwheel.com:4443\\\""' \
           APP_HTTPSSERVER='"\\\"http://api.objectwheel.com:8080\\\""' \
           APP_GITVER='"\\\"$$GIT_VERSION\\\""' \
           APP_GITHASH='"\\\"$$GIT_HASH\\\""' \
           APP_GITDATE='"\\\"$$GIT_DATE\\\""'

macx {
    interpreter.files = $$OUT_PWD/contrib/objectwheel-interpreter/objectwheel-interpreter
    interpreter.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += interpreter
}

!macx {
    interpreter.files = $$OUT_PWD/contrib/objectwheel-interpreter/objectwheel-interpreter
    interpreter.path = $$OUT_PWD/
    INSTALLS += interpreter
}