### Library Settings
TEMPLATE = lib
CONFIG  += plugin
TARGET   = utils
QT      += widgets network qml quick websockets multimedia
DEFINES += UTILS_LIBRARY

### Sources
HEADERS += $$PWD/zipper.h \
           $$PWD/delayer.h \
           $$PWD/saveutils.h \
           $$PWD/filemanager.h \
           $$PWD/parserutils.h \
           $$PWD/qmlformatter.h \
           $$PWD/previewresult.h \
           $$PWD/utils_global.h

SOURCES += $$PWD/zipper.cpp \
           $$PWD/delayer.cpp \
           $$PWD/saveutils.cpp \
           $$PWD/parserutils.cpp \
           $$PWD/filemanager.cpp \
           $$PWD/qmlformatter.cpp \
           $$PWD/previewresult.cpp

###Includes
include($$PWD/fit/fit.pri)
include($$PWD/miniz/miniz.pri)
include($$PWD/qmlparser/qmlparser.pri)
include($$PWD/components/components.pri)
