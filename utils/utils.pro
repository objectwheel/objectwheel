### Library Settings
TEMPLATE = lib
CONFIG  += plugin c++14
TARGET   = utils
QT      += widgets qml quick websockets multimedia concurrent
DEFINES += UTILS_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

### Sources
HEADERS += \
    $$PWD/delayer.h \
    $$PWD/saveutils.h \
    $$PWD/filemanager.h \
    $$PWD/parserutils.h \
    $$PWD/qmlformatter.h \
    $$PWD/previewresult.h \
    $$PWD/hashfactory.h \
    $$PWD/utils_global.h \
    $$PWD/quicktheme.h \
    $$PWD/previewercommands.h \
    $$PWD/paintutils.h \
    $$PWD/appfontsettings.h \
    $$PWD/utilityfunctions.h \
    $$PWD/offlinestorage.h

SOURCES += \
    $$PWD/delayer.cpp \
    $$PWD/saveutils.cpp \
    $$PWD/parserutils.cpp \
    $$PWD/filemanager.cpp \
    $$PWD/qmlformatter.cpp \
    $$PWD/hashfactory.cpp \
    $$PWD/quicktheme.cpp \
    $$PWD/paintutils.cpp \
    $$PWD/appfontsettings.cpp \
    $$PWD/utilityfunctions.cpp \
    $$PWD/offlinestorage.cpp

RESOURCES += \
    $$PWD/resources/internal/internal.qrc \
    $$PWD/resources/shared/moduleresolver/moduleresolver.qrc

###Includes
include($$PWD/qmlparser/qmlparser.pri)
include($$PWD/components/components.pri)