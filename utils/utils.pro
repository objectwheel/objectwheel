### Library Settings
TEMPLATE = lib
CONFIG  += plugin c++14 strict_c++
TARGET   = Utils
QT      += core-private widgets-private widgets qml quick websockets multimedia quickcontrols2
DEFINES += UTILS_LIBRARY
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

### Sources
HEADERS += \
    $$PWD/cleanexit.h \
    $$PWD/delayer.h \
    $$PWD/saveutils.h \
    $$PWD/toolutils.h \
    $$PWD/parserutils.h \
    $$PWD/renderinfo.h \
    $$PWD/hashfactory.h \
    $$PWD/utils_global.h \
    $$PWD/quicktheme.h \
    $$PWD/renderercommands.h \
    $$PWD/paintutils.h \
    $$PWD/utilityfunctions.h \
    $$PWD/filesystemutils.h \
    $$PWD/tooltipwidget.h \
    $$PWD/textimagehandler.h \
    $$PWD/qmlerror.h

SOURCES += \
    $$PWD/cleanexit.cpp \
    $$PWD/delayer.cpp \
    $$PWD/saveutils.cpp \
    $$PWD/toolutils.cpp \
    $$PWD/parserutils.cpp \
    $$PWD/hashfactory.cpp \
    $$PWD/quicktheme.cpp \
    $$PWD/paintutils.cpp \
    $$PWD/utilityfunctions.cpp \
    $$PWD/filesystemutils.cpp \
    $$PWD/textimagehandler.cpp

RESOURCES += $$PWD/resources/resources.qrc

###Includes
include($$PWD/../objectwheel.pri)
include($$PWD/zipasync/zipasync.pri)
include($$PWD/platform/platform.pri)
include($$PWD/qmlparser/qmlparser.pri)
