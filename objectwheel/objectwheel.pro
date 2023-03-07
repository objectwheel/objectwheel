### App Settings
CONFIG  += c++14 strict_c strict_c++ utf8_source hide_symbols
TARGET   = Objectwheel
gcc:QMAKE_CXXFLAGS += -pedantic-errors
msvc:QMAKE_CXXFLAGS += -permissive-
windows:LIBS += -lWs2_32 -luser32 # PayloadManager uses setsockopt()
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
#DEFINES += RENDERER_DEBUG
#DEFINES += PAYLOADMANAGER_DEBUG

QT += network websockets help widgets widgets-private

### Includes
include($$PWD/../app.pri)
include($$PWD/install.pri)
include($$PWD/run/run.pri)
include($$PWD/toolbox/toolbox.pri)
include($$PWD/modeselector/modeselector.pri)
include($$PWD/properties/properties.pri)
include($$PWD/controls/controls.pri)
include($$PWD/forms/forms.pri)
include($$PWD/builds/builds.pri)
include($$PWD/codeeditor/codeeditor.pri)
include($$PWD/core/core.pri)
include($$PWD/designer/designer.pri)
include($$PWD/help/help.pri)
include($$PWD/managers/managers.pri)
include($$PWD/misc/misc.pri)
include($$PWD/options/options.pri)
include($$PWD/assets/assets.pri)
include($$PWD/output/output.pri)
include($$PWD/render/render.pri)
include($$PWD/welcome/welcome.pri)
include($$PWD/subscription/subscription.pri)
include($$PWD/widgets/widgets.pri)
include($$PWD/windows/windows.pri)
include($$PWD/settings/settings.pri)
include($$PWD/platform/platform.pri)

RESOURCES += $$PWD/resources/resources.qrc

BEFORE  = $$re_escape($$find(DEFINES, APP_LABEL.*))
AFTER   = APP_LABEL='"\\\"$$TARGET (Beta)\\\""'
DEFINES = $$replace(DEFINES, $$BEFORE, $$AFTER)
