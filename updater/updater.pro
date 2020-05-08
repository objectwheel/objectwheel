### App Settings
CONFIG  -= app_bundle
CONFIG  += console c++14 strict_c++
TARGET   = Updater
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
unix:QMAKE_CXXFLAGS += -pedantic-errors

QT -= gui

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

SOURCES += $$PWD/main.cpp

### Utils Settings
include($$PWD/../objectwheel.pri)
