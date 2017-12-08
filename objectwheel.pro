VERSION  = 2.0
TEMPLATE = app
TARGET   = Objectwheel
DEFINES += QT_QML_DEBUG_NO_WARNING
QT      += widgets network qml quick quickcontrols2 quickwidgets webview websockets multimedia sensors svg concurrent

include($$PWD/contrib/contrib.pri)
include($$PWD/src/src.pri)
