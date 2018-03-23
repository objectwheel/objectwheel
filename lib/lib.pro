###Includes
include($$PWD/fit/fit.pri)
include($$PWD/miniz/miniz.pri)
include($$PWD/qmlparser/qmlparser.pri)
include($$PWD/components/components.pri)

### Library Settings
TEMPLATE = lib
CONFIG  += plugin
TARGET   = objectwheel
QT      += gui widgets qml network websockets multimedia

### Sources
HEADERS += $$PWD/zipper.h \
           $$PWD/delayer.h \
           $$PWD/saveutils.h \
           $$PWD/filemanager.h \
           $$PWD/parserutils.h \
           $$PWD/qmlformatter.h \
           $$PWD/previewresult.h

SOURCES += $$PWD/zipper.cpp \
           $$PWD/delayer.cpp \
           $$PWD/saveutils.cpp \
           $$PWD/parserutils.cpp \
           $$PWD/filemanager.cpp \
           $$PWD/qmlformatter.cpp \
           $$PWD/previewresult.cpp