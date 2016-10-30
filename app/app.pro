VERSION          = 1.0
QT              += core gui widgets network qml quick quickwidgets
TEMPLATE         = app
CONFIG          += c++11
TARGET           = DemoApp

CONFIG(debug, debug|release) {
	BUILD_DIR = $$PWD/../build/$$first(QMAKE_PLATFORM)-$$QT_ARCH-debug
} else {
	BUILD_DIR = $$PWD/../build/$$first(QMAKE_PLATFORM)-$$QT_ARCH-release
}

DESTDIR          = $$BUILD_DIR
OBJECTS_DIR      = $$BUILD_DIR/.obj
MOC_DIR          = $$BUILD_DIR/.moc
RCC_DIR          = $$BUILD_DIR/.rcc
UI_DIR           = $$BUILD_DIR/.ui

QMAKE_DISTCLEAN += -r $$PWD/../build

include($$PWD/app.pri)
include($$PWD/src/src.pri);
