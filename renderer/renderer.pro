### Includes
include($$PWD/platform/platform.pri)

### App Settings
TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  -= qtquickcompiler
CONFIG  += console c++14 strict_c++
TARGET   = Renderer
DEFINES += QT_QML_DEBUG_NO_WARNING
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
#DEFINES += RENDERER_DEBUG

QT += widgets network qml quick datavisualization
QT += sensors svg scxml purchasing positioning nfc
QT += charts multimedia bluetooth gamepad location
QT += quick-private quicktemplates2-private websockets

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

SOURCES += $$PWD/main.cpp \
           $$PWD/rendersocket.cpp \
           $$PWD/applicationcore.cpp \
           $$PWD/renderengine.cpp \
           $$PWD/commanddispatcher.cpp \
           $$PWD/commandlineparser.cpp \
           $$PWD/renderutils.cpp \
           $$PWD/knownenums.cpp

HEADERS += $$PWD/rendersocket.h \
           $$PWD/applicationcore.h \
           $$PWD/renderengine.h \
           $$PWD/commanddispatcher.h \
           $$PWD/commandlineparser.h \
           $$PWD/renderutils.h \
           $$PWD/knownenums.h


### Utils Settings
include($$PWD/../objectwheel.pri)
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/zipasync
INCLUDEPATH += $$PWD/../utils/zipasync/async
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
