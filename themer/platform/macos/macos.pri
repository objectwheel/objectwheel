INCLUDEPATH += $$PWD
LIBS += -framework AppKit
ICON = $$PWD/icon.icns
OTHER_FILES += $$PWD/icon.icns
QMAKE_INFO_PLIST = $$PWD/Info.plist
QMAKE_LFLAGS += -Wl,-sectcreate,__TEXT,__info_plist,$$system_quote($$PWD/Info.plist)
HEADERS += $$PWD/macoperations.h
SOURCES += $$PWD/macoperations.mm
