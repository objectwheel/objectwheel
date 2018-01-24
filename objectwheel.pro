TEMPLATE = app
TARGET = Objectwheel
QT += widgets network qml quick quickcontrols2 quickwidgets webview webenginecore webchannel webenginewidgets websockets multimedia sensors svg concurrent

include($$PWD/gitversion.pri)
include($$PWD/src/src.pri)
include($$PWD/contrib/contrib.pri)

DEFINES += QT_QML_DEBUG_NO_WARNING \
           APP_VER='"\\\"$$VERSION\\\""' \
           APP_NAME='"\\\"$$TARGET\\\""' \
           APP_CORP='"\\\"Objectwheel, Inc.\\\""' \
           APP_DOMAIN='"\\\"objectwheel.com\\\""' \
           APP_SERVER='"\\\"wss://localhost:4443\\\""' \
           APP_GITVER='"\\\"$$GIT_VERSION\\\""' \
           APP_GITHASH='"\\\"$$GIT_HASH\\\""' \
           APP_GITDATE='"\\\"$$GIT_DATE\\\""'
