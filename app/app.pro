### App Settings
TEMPLATE = app
CONFIG  += c++14
CONFIG  -= qtquickcompiler
TARGET   = Objectwheel
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
#DEFINES += PREVIEWER_DEBUG

QT += quickwidgets concurrent quick-private qml-private
QT += network qml quick websockets help widgets widgets-private

### Includes
include($$PWD/app.pri)
include($$PWD/aes/aes.pri)
include($$PWD/builds/builds.pri)
include($$PWD/codeeditor/codeeditor.pri)
include($$PWD/core/core.pri)
include($$PWD/help/help.pri)
include($$PWD/managers/managers.pri)
include($$PWD/misc/misc.pri)
include($$PWD/options/options.pri)
include($$PWD/panes/panes.pri)
include($$PWD/previewerlink/previewerlink.pri)
include($$PWD/welcome/welcome.pri)
include($$PWD/widgets/widgets.pri)
include($$PWD/windows/windows.pri)
include($$PWD/settings/settings.pri)
include($$PWD/resources/resources.pri)
include($$PWD/platform/platform.pri)
include($$PWD/zipasync/zipasync.pri)