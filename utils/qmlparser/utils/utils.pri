include($$PWD/tasking/tasking.pri)
include($$PWD/tooltip/tooltip.pri)

SOURCES += \
    $$PWD/id.cpp \
    $$PWD/futuresynchronizer.cpp \
    $$PWD/filepath.cpp \
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
    $$PWD/namevaluedictionary.cpp \
    $$PWD/searchresultitem.cpp \
    $$PWD/stringutils.cpp \
    $$PWD/multitextcursor.cpp \
    $$PWD/namevalueitem.cpp \
    $$PWD/globalfilechangeblocker.cpp \
    $$PWD/devicefileaccess.cpp \
    $$PWD/guard.cpp \
    $$PWD/commandline.cpp \
    $$PWD/camelcasecursor.cpp \
    $$PWD/macroexpander.cpp \
    $$PWD/uncommentselection.cpp

HEADERS += \
    $$PWD/id.h \
    $$PWD/macroexpander.h \
    $$PWD/commandline.h \
    $$PWD/guard.h \
    $$PWD/globalfilechangeblocker.h \
    $$PWD/devicefileaccess.h \
    $$PWD/camelcasecursor.h \
    $$PWD/searchresultitem.h \
    $$PWD/stringutils.h \
    $$PWD/multitextcursor.h \
    $$PWD/namevalueitem.h \
    $$PWD/namevaluedictionary.h \
    $$PWD/utilstr.h \
    $$PWD/environmentfwd.h \
    $$PWD/predicates.h \
    $$PWD/futuresynchronizer.h \
    $$PWD/expected.h \
    $$PWD/expected.hpp \
    $$PWD/filepath.h \
    $$PWD/filepathinfo.h \
    $$PWD/osspecificaspects.h \
    $$PWD/utiltypes.h \
    $$PWD/fileutils.h \
    $$PWD/functiontraits.h \
    $$PWD/hostosinfo.h \
    $$PWD/json.h \
    $$PWD/osspecificaspects.h \
    $$PWD/qtcassert.h \
    $$PWD/runextensions.h \
    $$PWD/savefile.h \
    $$PWD/changeset.h \
    $$PWD/fuzzymatcher.h \
    $$PWD/textutils.h \
    $$PWD/faketooltip.h \
    $$PWD/environment.h \
    $$PWD/filesystemwatcher.h \
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
