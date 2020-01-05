### Includes
include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)

### App Settings
TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  -= qtquickcompiler
CONFIG  += console c++14 strict_c++
TARGET   = Themer
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

QT += gui widgets qml quick

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

SOURCES += $$PWD/main.cpp

### Utils Settings
include($$PWD/../objectwheel.pri)
INCLUDEPATH += $$PWD/../utils
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
