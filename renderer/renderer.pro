### Includes
include($$PWD/platform/platform.pri)

### App Settings
TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  += console c++14 strict_c++
TARGET   = renderer
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
#DEFINES += RENDERER_DEBUG

QT += quickwidgets webenginewidgets quick-private qml-private quicktemplates2-private
QT += network qml quick webview websockets gamepad webchannel
QT += sensors svg scxml purchasing positioning nfc location bluetooth datavisualization webengine
QT += charts 3dcore 3drender 3dinput 3dlogic 3dextras 3danimation 3dquick multimedia webenginecore    

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

DEFINES += QT_QML_DEBUG_NO_WARNING

SOURCES += $$PWD/main.cpp \
           $$PWD/rendersocket.cpp \
           $$PWD/applicationcore.cpp \
           $$PWD/renderengine.cpp \
           $$PWD/commanddispatcher.cpp \
           $$PWD/commandlineparser.cpp \
           $$PWD/renderutils.cpp

HEADERS += $$PWD/rendersocket.h \
           $$PWD/applicationcore.h \
           $$PWD/renderengine.h \
           $$PWD/commanddispatcher.h \
           $$PWD/commandlineparser.h \
           $$PWD/renderutils.h


### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/zipasync
INCLUDEPATH += $$PWD/../utils/zipasync/async
INCLUDEPATH += $$PWD/../utils/components
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
