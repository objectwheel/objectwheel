### Library Settings
TEMPLATE = lib
CONFIG  += plugin c++14 strict_c++
TARGET   = utils
QT      += core-private widgets-private widgets qml quick websockets multimedia quickcontrols2
DEFINES += UTILS_LIBRARY
DEFINES += QT_QML_DEBUG_NO_WARNING
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

### Sources
HEADERS += \
    $$PWD/delayer.h \
    $$PWD/saveutils.h \
    $$PWD/parserutils.h \
    $$PWD/qmlformatter.h \
    $$PWD/renderinfo.h \
    $$PWD/hashfactory.h \
    $$PWD/utils_global.h \
    $$PWD/quicktheme.h \
    $$PWD/renderercommands.h \
    $$PWD/paintutils.h \
    $$PWD/utilityfunctions.h \
    $$PWD/filesystemutils.h \
    $$PWD/serializeenum.h \
    $$PWD/qmlerror.h \
    $$PWD/textimagehandler.h

SOURCES += \
    $$PWD/delayer.cpp \
    $$PWD/saveutils.cpp \
    $$PWD/parserutils.cpp \
    $$PWD/qmlformatter.cpp \
    $$PWD/hashfactory.cpp \
    $$PWD/quicktheme.cpp \
    $$PWD/paintutils.cpp \
    $$PWD/utilityfunctions.cpp \
    $$PWD/filesystemutils.cpp \
    $$PWD/qmlerror.cpp \
    $$PWD/textimagehandler.cpp

RESOURCES += \
    $$PWD/resources/internal/internal.qrc \
    $$PWD/resources/shared/tools/tools.qrc \
    $$PWD/resources/shared/moduleresolver/moduleresolver.qrc

###Includes
include($$PWD/../objectwheel.pri)
include($$PWD/zipasync/zipasync.pri)
include($$PWD/platform/platform.pri)
include($$PWD/qmlparser/qmlparser.pri)
include($$PWD/components/components.pri)