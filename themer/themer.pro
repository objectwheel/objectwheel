### Includes
include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)

### App Settings
TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  += console c++14 strict_c++
TARGET   = themer
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

QT += gui widgets qml quick

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

DEFINES += QT_QML_DEBUG_NO_WARNING

SOURCES += $$PWD/main.cpp

### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/components
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
