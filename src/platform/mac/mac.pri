QT += macextras
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

ICON = $$PWD/icon.icns
QMAKE_MAC_SDK = macosx10.10
QMAKE_INFO_PLIST = $$PWD/Info.plist
QMAKE_OBJCXXFLAGS += -fobjc-arc

LIBS += -framework AppKit
HEADERS += $$PWD/mactoolbar.h
OBJECTIVE_SOURCES += $$PWD/mactoolbar.mm