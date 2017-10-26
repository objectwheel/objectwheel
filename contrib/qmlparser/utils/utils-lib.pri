dll {
    DEFINES += QTCREATOR_UTILS_LIB
} else {
    DEFINES += QTCREATOR_UTILS_STATIC_LIB
}

#QT += gui network qml
#CONFIG += exceptions # used by portlist.cpp, textfileformat.cpp, and ssh/*
#win32: LIBS += -luser32 -lshell32
# PortsGatherer
#win32: LIBS += -liphlpapi -lws2_32

SOURCES += $$PWD/fileutils.cpp \
    $$PWD/hostosinfo.cpp \
    $$PWD/json.cpp \
    $$PWD/qtcassert.cpp \
    $$PWD/runextensions.cpp \
    $$PWD/savefile.cpp \
    $$PWD/changeset.cpp


HEADERS += \
    $$PWD/fileutils.h \
    $$PWD/functiontraits.h \
    $$PWD/hostosinfo.h \
    $$PWD/json.h \
    $$PWD/osspecificaspects.h \
    $$PWD/qtcassert.h \
    $$PWD/runextensions.h \
    $$PWD/utils_global.h \
    $$PWD/savefile.h \
    $$PWD/changeset.h

osx {
    HEADERS += \
        $$PWD/fileutils_mac.h
    OBJECTIVE_SOURCES += \
        $$PWD/fileutils_mac.mm
    LIBS += -framework Foundation
}
