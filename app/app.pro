include($$PWD/app.pri)

VERSION         = 1.0
QT             += core gui widgets qml quick quickwidgets network
TEMPLATE        = app
CONFIG         += c++11 static

debug {
    DESTDIR     = $$PWD/../build/debug
    OBJECTS_DIR = $$PWD/../build/debug/.obj
    MOC_DIR     = $$PWD/../build/debug/.moc
    RCC_DIR     = $$PWD/../build/debug/.rcc
    UI_DIR      = $$PWD/../build/debug/.ui
} release {
    DESTDIR     = $$PWD/../build/release
    OBJECTS_DIR = $$PWD/../build/release/.obj
    MOC_DIR     = $$PWD/../build/release/.moc
    RCC_DIR     = $$PWD/../build/release/.rcc
    UI_DIR      = $$PWD/../build/release/.ui
}

QMAKE_DISTCLEAN += -r $$PWD/../build

include($$PWD/src/src.pri);