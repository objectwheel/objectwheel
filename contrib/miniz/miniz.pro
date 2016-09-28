VERSION         = 1.0
CONFIG         -= qt
CONFIG         += static
QT             -= gui core
TEMPLATE        = lib
INCLUDEPATH     = $$PWD/include

debug {
    DESTDIR     = $$PWD/build/debug
    OBJECTS_DIR = $$PWD/build/debug/.obj
    MOC_DIR     = $$PWD/build/debug/.moc
    RCC_DIR     = $$PWD/build/debug/.rcc
    UI_DIR      = $$PWD/build/debug/.ui
} release {
    DESTDIR     = $$PWD/build/release
    OBJECTS_DIR = $$PWD/build/release/.obj
    MOC_DIR     = $$PWD/build/release/.moc
    RCC_DIR     = $$PWD/build/release/.rcc
    UI_DIR      = $$PWD/build/release/.ui
}

QMAKE_DISTCLEAN += -r $$PWD/build

include($$PWD/src/src.pri);