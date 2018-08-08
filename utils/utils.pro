### Library Settings
TEMPLATE = lib
CONFIG  += plugin c++14
TARGET   = utils
QT      += widgets network qml quick websockets multimedia
DEFINES += UTILS_LIBRARY
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

### Sources
HEADERS += \
    $$PWD/zipper.h \
    $$PWD/delayer.h \
    $$PWD/saveutils.h \
    $$PWD/filemanager.h \
    $$PWD/parserutils.h \
    $$PWD/qmlformatter.h \
    $$PWD/previewresult.h \
    $$PWD/hashfactory.h \
    $$PWD/utils_global.h \
    $$PWD/appfontsettings.h \
    $$PWD/quicktheme.h \
    $$PWD/previewercommands.h \
    wfw.h \
    paintutils.h

SOURCES += \
    $$PWD/zipper.cpp \
    $$PWD/delayer.cpp \
    $$PWD/saveutils.cpp \
    $$PWD/parserutils.cpp \
    $$PWD/filemanager.cpp \
    $$PWD/qmlformatter.cpp \
    $$PWD/hashfactory.cpp \
    $$PWD/appfontsettings.cpp \
    $$PWD/quicktheme.cpp \
    paintutils.cpp

RESOURCES += \
    $$PWD/sharedresources/sharedresources.qrc \
    $$PWD/resources/resources.qrc

###Includes
include($$PWD/miniz/miniz.pri)
include($$PWD/qmlparser/qmlparser.pri)
include($$PWD/components/components.pri)