### App Settings
CONFIG  += cmdline c++17 strict_c strict_c++ utf8_source hide_symbols
TARGET   = Themer
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060502

QT += widgets qml quick

### Sources
INCLUDEPATH += $$PWD
SOURCES += $$PWD/main.cpp
RESOURCES += $$PWD/resources/resources.qrc

### Includes
include($$PWD/../app.pri)
include($$PWD/platform/platform.pri)

### Utils Settings
INCLUDEPATH += $$PWD/../utils
LIBS        += -L$$OUT_PWD/../utils -lUtils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
