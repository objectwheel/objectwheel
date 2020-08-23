### Library Settings
TEMPLATE = lib
CONFIG  += plugin c++14 strict_c strict_c++ utf8_source hide_symbols skip_target_version_ext
TARGET   = Utils
QT      += core-private widgets-private widgets qml quick websockets multimedia quickcontrols2
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
           UTILS_LIBRARY \
           ASYNC_LIBRARY \
           ZIPASYNC_LIBRARY \
           FASTDOWNLOADER_LIBRARY

INCLUDEPATH += $$PWD

### Sources
HEADERS += \
    $$PWD/appconstants.h \
    $$PWD/signalwatcher.h \
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
    $$PWD/qmlerror.h \
    $$PWD/planinfo.h

SOURCES += \
    $$PWD/signalwatcher.cpp \
    $$PWD/delayer.cpp \
    $$PWD/saveutils.cpp \
    $$PWD/toolutils.cpp \
    $$PWD/parserutils.cpp \
    $$PWD/hashfactory.cpp \
    $$PWD/quicktheme.cpp \
    $$PWD/paintutils.cpp \
    $$PWD/utilityfunctions.cpp \
    $$PWD/filesystemutils.cpp \
    $$PWD/textimagehandler.cpp \
    $$PWD/planinfo.cpp

RESOURCES += $$PWD/resources/resources.qrc

###Includes
include($$PWD/zipasync/zipasync.pri)
include($$PWD/fastdownloader/fastdownloader.pri)
include($$PWD/platform/platform.pri)
include($$PWD/qmlparser/qmlparser.pri)
