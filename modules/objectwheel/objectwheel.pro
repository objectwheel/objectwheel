TEMPLATE = lib
CONFIG  += plugin c++14 strict_c++
TARGET   = objectwheelmodule
DESTDIR  = ../Modules/Objectwheel
QT      += qml
DEFINES += QT_QML_DEBUG_NO_WARNING
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

OW_MODULE_VERSION_MAJOR = 1
OW_MODULE_VERSION_MINOR = 0

DEFINES += OW_MODULE_VERSION_MAJOR=$$OW_MODULE_VERSION_MAJOR \
           OW_MODULE_VERSION_MINOR=$$OW_MODULE_VERSION_MINOR
VERSION  = $${OW_MODULE_VERSION_MAJOR}.$${OW_MODULE_VERSION_MINOR}

INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

HEADERS += $$PWD/objectwheel.h \
           $$PWD/translation.h

SOURCES += $$PWD/objectwheelmodule.cpp \
           $$PWD/objectwheel.cpp \
           $$PWD/translation.cpp

OTHER_FILES += $$PWD/qmldir

include($$PWD/../modules.pri)
