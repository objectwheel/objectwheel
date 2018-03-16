TEMPLATE = app
TARGET = Objectwheel

QT += quickwidgets qml quick help
QT += widgets network websockets svg concurrent quickcontrols2
QT += webenginewidgets webchannel webengine webenginecore

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
    previewer.files = $$OUT_PWD/contrib/objectwheel-previewer/objectwheel-previewer
    previewer.path = Contents/MacOS
    docs.files = $$PWD/src/resources/docs/*
    docs.path = Contents/MacOS/docs
    QMAKE_BUNDLE_DATA += interpreter previewer
} else {
    interpreter.files = $$OUT_PWD/contrib/objectwheel-interpreter/objectwheel-interpreter
    interpreter.path = $$OUT_PWD/
    previewer.files = $$OUT_PWD/contrib/objectwheel-previewer/objectwheel-previewer
    previewer.path = $$OUT_PWD/
    docs.files = $$PWD/src/resources/docs/*
    docs.path = $$OUT_PWD/docs
    INSTALLS += interpreter previewer docs
}