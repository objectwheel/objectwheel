INCLUDEPATH += $$PWD
include($$PWD/qmleditor/qmleditor.pri)

HEADERS += $$PWD/blockdata.h \
           $$PWD/bracketband.h \
           $$PWD/linenumberband.h \
           $$PWD/markband.h \
           $$PWD/qmlcodedocument.h \
           $$PWD/qmlcodeeditor.h \
           $$PWD/qmlcodeeditortoolbar.h \
           $$PWD/qmlcodeeditorwidget.h \
           $$PWD/rowbar.h

SOURCES += $$PWD/bracketband.cpp \
           $$PWD/linenumberband.cpp \
           $$PWD/markband.cpp \
           $$PWD/qmlcodedocument.cpp \
           $$PWD/qmlcodeeditor.cpp \
           $$PWD/qmlcodeeditortoolbar.cpp \
           $$PWD/qmlcodeeditorwidget.cpp \
           $$PWD/rowbar.cpp