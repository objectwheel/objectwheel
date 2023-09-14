### App Settings
CONFIG  += cmdline c++17 strict_c strict_c++ utf8_source hide_symbols
TARGET   = Updater
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060502

QT += widgets

### Sources
INCLUDEPATH += $$PWD
SOURCES += $$PWD/main.cpp
RESOURCES += $$PWD/resources/resources.qrc

### Includes
include($$PWD/../app.pri)
include($$PWD/platform/platform.pri)

### Utils Settings
INCLUDEPATH += $$PWD/../utils
