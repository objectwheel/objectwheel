VERSION          = 1.0
QT              += core gui widgets network qml quick quickwidgets webview websockets multimedia sensors svg concurrent
TEMPLATE         = app
CONFIG          += c++11
TARGET           = objectwheel
PROJECT_ROOT     = $$clean_path($$PWD/..)
RC_ICONS         = $$PWD/owicon.ico

include($$PWD/detect_build.pri) # BUILD_POSTFIX

!ios {
    BUILD_DIR        = $$PROJECT_ROOT/$$BUILD_POSTFIX
    DESTDIR          = $$BUILD_DIR
    OBJECTS_DIR      = $$BUILD_DIR/.obj
    MOC_DIR          = $$BUILD_DIR/.moc
    RCC_DIR          = $$BUILD_DIR/.rcc
    UI_DIR           = $$BUILD_DIR/.ui
    QMAKE_DISTCLEAN += -r $$PROJECT_ROOT/build
}

include($$PWD/contrib.pri)
include($$PWD/src/src.pri);
