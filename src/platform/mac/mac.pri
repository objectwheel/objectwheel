QT += macextras
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

ICON = $$PWD/icon.icns
QMAKE_MAC_SDK = macosx10.10
QMAKE_INFO_PLIST = $$PWD/Info.plist
LIBS += -framework Cocoa -framework QuartzCore

HEADERS += $$PWD/mactoolbar.h
SOURCES += $$PWD/mactoolbar.mm