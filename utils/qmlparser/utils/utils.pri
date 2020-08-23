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

macos {
    HEADERS += $$PWD/fileutils_mac.h
    SOURCES += $$PWD/fileutils_mac.mm
    LIBS += -framework Foundation
}
