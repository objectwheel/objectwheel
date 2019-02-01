ICON = $$PWD/icon.icns
QMAKE_INFO_PLIST = $$PWD/Info.plist

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
LIBS += -framework AppKit

HEADERS += $$PWD/windowoperations.h
SOURCES += $$PWD/windowoperations.mm

