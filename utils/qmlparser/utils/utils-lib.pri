dll {
    DEFINES += QTCREATOR_UTILS_LIB
} else {
    DEFINES += QTCREATOR_UTILS_STATIC_LIB
}

#QT += gui network qml
#CONFIG += exceptions # used by portlist.cpp, textfileformat.cpp, and ssh/*
win32: LIBS += -lshell32
# PortsGatherer
#win32: LIBS += -liphlpapi -lws2_32

include($$PWD/tooltip/tooltip.pri)

SOURCES += \
    $$PWD/fileutils.cpp \
    $$PWD/hostosinfo.cpp \
    $$PWD/json.cpp \
    $$PWD/filesearch.cpp \
    $$PWD/qtcassert.cpp \
    $$PWD/runextensions.cpp \
    $$PWD/savefile.cpp \
    $$PWD/changeset.cpp \
    $$PWD/fuzzymatcher.cpp \
    $$PWD/textutils.cpp \
    $$PWD/faketooltip.cpp \
    $$PWD/environment.cpp \
    $$PWD/filesystemwatcher.cpp \
    $$PWD/htmldocextractor.cpp \
    $$PWD/qtcolorbutton.cpp \
    $$PWD/uncommentselection.cpp

HEADERS += \
    $$PWD/fileutils.h \
    $$PWD/optional.h \
    $$PWD/linecolumn.h \
    $$PWD/functiontraits.h \
    $$PWD/hostosinfo.h \
    $$PWD/json.h \
    $$PWD/osspecificaspects.h \
    $$PWD/qtcassert.h \
    $$PWD/runextensions.h \
    $$PWD/utils_global.h \
    $$PWD/savefile.h \
    $$PWD/changeset.h \
    $$PWD/declarationmacros.h \
    $$PWD/fuzzymatcher.h \
    $$PWD/textutils.h \
    $$PWD/faketooltip.h \
    $$PWD/environment.h \
    $$PWD/filesystemwatcher.h \
    $$PWD/executeondestruction.h \
    $$PWD/htmldocextractor.h \
    $$PWD/link.h \
    $$PWD/settingsutils.h \
    $$PWD/filesearch.h \
    $$PWD/mapreduce.h \
    $$PWD/qtcolorbutton.h \
    $$PWD/uncommentselection.h

macx {
    HEADERS += \
        $$PWD/fileutils_mac.h
    OBJECTIVE_SOURCES += \
        $$PWD/fileutils_mac.mm
    LIBS += -framework Foundation
}
