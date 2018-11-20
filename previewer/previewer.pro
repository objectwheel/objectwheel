### Includes
include($$PWD/platform/platform.pri)

### App Settings
TEMPLATE = app
CONFIG  -= app_bundle
CONFIG  += console c++14
TARGET   = previewer
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
#DEFINES += PREVIEWER_DEBUG

QT += quickwidgets webenginewidgets concurrent quick-private qml-private
QT += network qml quick webview websockets gamepad webchannel
QT += sensors svg scxml purchasing positioning nfc location bluetooth datavisualization webengine
QT += charts 3dcore 3drender 3dinput 3dlogic 3dextras 3danimation 3dquick multimedia webenginecore    

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

DEFINES += QT_QML_DEBUG_NO_WARNING

SOURCES += $$PWD/main.cpp \
           $$PWD/previewersocket.cpp \
           $$PWD/applicationcore.cpp \
           $$PWD/bootsettings.cpp \
           $$PWD/previewer.cpp \
           $$PWD/commanddispatcher.cpp \
           $$PWD/commandlineparser.cpp \
           $$PWD/previewerutils.cpp

HEADERS += $$PWD/previewersocket.h \
           $$PWD/applicationcore.h \
           $$PWD/bootsettings.h \
           $$PWD/previewer.h \
           $$PWD/commanddispatcher.h \
           $$PWD/commandlineparser.h \
           $$PWD/previewerutils.h

### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/components
RESOURCES   += $$PWD/../utils/resources/shared/moduleresolver/moduleresolver.qrc
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug
