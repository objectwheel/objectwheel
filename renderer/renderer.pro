### App Settings
CONFIG  += cmdline c++17 strict_c strict_c++ utf8_source hide_symbols
TARGET   = Renderer
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060502
#DEFINES += RENDERER_DEBUG

QT += widgets network qml quick datavisualization
QT += sensors svg scxml positioning nfc
QT += charts multimedia bluetooth location
QT += quick-private quicktemplates2-private websockets

### Sources
INCLUDEPATH += $$PWD

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

RESOURCES += $$PWD/resources/resources.qrc

### Includes
include($$PWD/../app.pri)
include($$PWD/platform/platform.pri)

### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/zipasync
INCLUDEPATH += $$PWD/../utils/zipasync/async
LIBS        += -L$$OUT_PWD/../utils -lUtils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
