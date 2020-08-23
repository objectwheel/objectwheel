INCLUDEPATH += $$PWD

DEFINES += CORE_INCLUDE_STATIC
DEFINES += QMLEDITORWIDGETS_INCLUDE_STATIC
DEFINES += TEXTEDITOR_INCLUDE_STATIC
DEFINES += QMLJSEDITOR_INCLUDE_STATIC
DEFINES += QMLJSTOOLS_INCLUDE_STATIC

include($$PWD/texteditor/texteditor.pri)
include($$PWD/coreplugin/coreplugin.pri)
include($$PWD/qmljstools/qmljstools.pri)
include($$PWD/qmleditorwidgets/qmleditorwidgets.pri)
include($$PWD/qmljseditor/qmljseditor.pri)
