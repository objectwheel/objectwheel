### App Settings
CONFIG  += cmdline c++14 strict_c strict_c++ utf8_source hide_symbols
TARGET   = Updater
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

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
