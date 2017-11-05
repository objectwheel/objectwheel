VERSION          = 1.0
QT              += core gui widgets network qml quick quickwidgets webview websockets multimedia sensors svg concurrent
TEMPLATE         = app
CONFIG          += c++11
TARGET           = Objectwheel
PROJECT_ROOT     = $$clean_path($$PWD/..)
DEFINES         += QT_QML_DEBUG_NO_WARNING


include($$PWD/detect_build.pri) # BUILD_POSTFIX

qtHaveModule(opengl) : QT += opengl
!ios {
    BUILD_DIR        = $$PROJECT_ROOT/$$BUILD_POSTFIX
    DESTDIR          = $$BUILD_DIR
    OBJECTS_DIR      = $$BUILD_DIR/.obj
    MOC_DIR          = $$BUILD_DIR/.moc
    RCC_DIR          = $$BUILD_DIR/.rcc
    UI_DIR           = $$BUILD_DIR/.ui
    QMAKE_DISTCLEAN += -r $$PROJECT_ROOT/build
}

windows {
    RC_ICONS         = $$PWD/src/platform_spesific/windows/icon.ico
}

macx {
    QMAKE_MAC_SDK    = macosx10.10
    QMAKE_INFO_PLIST = $$PWD/src/platform_spesific/mac/Info.plist
    ICON             = $$PWD/src/platform_spesific/mac/icon.icns
#    QMAKE_POST_LINK += macdeployqt $$BUILD_DIR/Objectwheel.app/ -qmldir=$$PWD/src/resources/qmls/ -verbose=1 -dmg
}

include($$PWD/contrib.pri)
include($$PWD/src/src.pri);
