### App Settings
CONFIG  += cmdline c++14 strict_c strict_c++ utf8_source hide_symbols
TARGET   = Themer
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

QT += widgets qml quick

### Sources
INCLUDEPATH += $$PWD
SOURCES += $$PWD/main.cpp

### Includes
include($$PWD/../app.pri)
include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)

### Utils Settings
INCLUDEPATH += $$PWD/../utils
LIBS        += -L$$OUT_PWD/../utils -lUtils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
