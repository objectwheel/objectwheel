### App Settings
TEMPLATE = app
CONFIG  += c++14
TARGET   = Objectwheel
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
#DEFINES += PREVIEWER_DEBUG

QT += quickwidgets webenginewidgets concurrent quick-private qml-private widgets widgets-private
QT += network qml quick webview websockets gamepad webchannel quickcontrols2 help
QT += sensors svg scxml purchasing positioning nfc location bluetooth datavisualization webengine
QT += charts 3dcore 3drender 3dinput 3dlogic 3dextras 3danimation 3dquick multimedia webenginecore

### Includes
include($$PWD/version.pri)
include($$PWD/aes/aes.pri)
include($$PWD/miniz/miniz.pri)
include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)
include($$PWD/qmleditor/qmleditor.pri)

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

HEADERS += $$PWD/mainwindow.h \
           $$PWD/flatbutton.h \
           $$PWD/dirlocker.h \
           $$PWD/control.h \
           $$PWD/qmlcodeeditor.h \
           $$PWD/fileexplorer.h \
           $$PWD/filelist.h \
           $$PWD/toolboxtree.h \
           $$PWD/view.h \
           $$PWD/global.h \
           $$PWD/issuesbox.h \
           $$PWD/consolebox.h \
           $$PWD/searchbox.h \
           $$PWD/resizer.h \
           $$PWD/suppressor.h \
           $$PWD/platformswidget.h \
           $$PWD/androidwidget.h \
           $$PWD/moduleselectionwidget.h \
           $$PWD/build.h \
           $$PWD/androidcreatekeystorecertificate.h \
           $$PWD/downloadwidget.h \
           $$PWD/windowmanager.h \
           $$PWD/welcomewindow.h \
           $$PWD/aboutwindow.h \
           $$PWD/preferenceswindow.h \
           $$PWD/toolboxsettingswindow.h \
           $$PWD/menumanager.h \
           $$PWD/inspectorpane.h \
           $$PWD/formspane.h \
           $$PWD/propertiespane.h \
           $$PWD/toolboxpane.h \
           $$PWD/applicationcore.h \
           $$PWD/savemanager.h \
           $$PWD/projectmanager.h \
           $$PWD/toolmanager.h \
           $$PWD/usermanager.h \
           $$PWD/loadingbar.h \
           $$PWD/outputpane.h \
           $$PWD/registrationwidget.h \
           $$PWD/loginwidget.h \
           $$PWD/projectswidget.h \
           $$PWD/switch.h \
           $$PWD/bulkedit.h \
           $$PWD/focuslesslineedit.h \
           $$PWD/buttonslice.h \
           $$PWD/waitingspinnerwidget.h \
           $$PWD/authenticator.h \
           $$PWD/verificationwidget.h \
           $$PWD/countdown.h \
           $$PWD/robotwidget.h \
           $$PWD/forgetwidget.h \
           $$PWD/resetwidget.h \
           $$PWD/succeedwidget.h \
           $$PWD/projectdetailswidget.h \
           $$PWD/runmanager.h \
           $$PWD/controlpreviewingmanager.h \
           $$PWD/progressbar.h \
           $$PWD/centralwidget.h \
           $$PWD/designerwidget.h \
           $$PWD/runpane.h \
           $$PWD/pageswitcherpane.h \
           $$PWD/designerscene.h \
           $$PWD/designerview.h \
           $$PWD/qmlcodeeditorwidget.h \
           $$PWD/buildswidget.h \
           $$PWD/pages.h \
           $$PWD/projectoptionswidget.h \
           $$PWD/controlcreationmanager.h \
           $$PWD/helpwidget.h \
           $$PWD/webenginehelpviewer.h \
           $$PWD/helpviewer.h \
           $$PWD/form.h \
           $$PWD/themechooserwidget.h \
           $$PWD/scalingwidget.h \
           $$PWD/projecttemplateswidget.h \
           $$PWD/rowbar.h \
           $$PWD/textutils.h \
           $$PWD/linenumberband.h \
           $$PWD/breakpointband.h \
           $$PWD/bracketband.h \
           $$PWD/blockdata.h \
           $$PWD/qmlcodedocument.h \
           $$PWD/documentmanager.h \
           $$PWD/markband.h \
           $$PWD/toolbar.h \
           $$PWD/toolbutton.h \
           $$PWD/controlremovingmanager.h \
           $$PWD/projectexposingmanager.h \
           $$PWD/bootsettings.h \
           $$PWD/previewerserver.h \
           $$PWD/commanddispatcher.h \
           $$PWD/controlpropertymanager.h \
           $$PWD/transparentstyle.h \
           $$PWD/applicationstyle.h \
           $$PWD/zipper.h \
    globalresourcespane.h

SOURCES += $$PWD/main.cpp\
           $$PWD/mainwindow.cpp \
           $$PWD/flatbutton.cpp \
           $$PWD/dirlocker.cpp \
           $$PWD/control.cpp \
           $$PWD/qmlcodeeditor.cpp \
           $$PWD/fileexplorer.cpp \
           $$PWD/filelist.cpp \
           $$PWD/toolboxtree.cpp \
           $$PWD/view.cpp \
           $$PWD/issuesbox.cpp \
           $$PWD/consolebox.cpp \
           $$PWD/searchbox.cpp \
           $$PWD/resizer.cpp \
           $$PWD/suppressor.cpp \
           $$PWD/platformswidget.cpp \
           $$PWD/androidwidget.cpp \
           $$PWD/moduleselectionwidget.cpp \
           $$PWD/build.cpp \
           $$PWD/androidcreatekeystorecertificate.cpp \
           $$PWD/downloadwidget.cpp \
           $$PWD/windowmanager.cpp \
           $$PWD/welcomewindow.cpp \
           $$PWD/aboutwindow.cpp \
           $$PWD/preferenceswindow.cpp \
           $$PWD/toolboxsettingswindow.cpp \
           $$PWD/menumanager.cpp \
           $$PWD/inspectorpane.cpp \
           $$PWD/formspane.cpp \
           $$PWD/propertiespane.cpp \
           $$PWD/toolboxpane.cpp \
           $$PWD/applicationcore.cpp \
           $$PWD/savemanager.cpp \
           $$PWD/projectmanager.cpp \
           $$PWD/toolmanager.cpp \
           $$PWD/usermanager.cpp \
           $$PWD/loadingbar.cpp \
           $$PWD/outputpane.cpp \
           $$PWD/registrationwidget.cpp \
           $$PWD/loginwidget.cpp \
           $$PWD/projectswidget.cpp \
           $$PWD/switch.cpp \
           $$PWD/bulkedit.cpp \
           $$PWD/focuslesslineedit.cpp \
           $$PWD/buttonslice.cpp \
           $$PWD/waitingspinnerwidget.cpp \
           $$PWD/authenticator.cpp \
           $$PWD/verificationwidget.cpp \
           $$PWD/countdown.cpp \
           $$PWD/robotwidget.cpp \
           $$PWD/forgetwidget.cpp \
           $$PWD/resetwidget.cpp \
           $$PWD/succeedwidget.cpp \
           $$PWD/projectdetailswidget.cpp \
           $$PWD/runmanager.cpp \
           $$PWD/progressbar.cpp \
           $$PWD/centralwidget.cpp \
           $$PWD/designerwidget.cpp \
           $$PWD/runpane.cpp \
           $$PWD/pageswitcherpane.cpp \
           $$PWD/designerscene.cpp \
           $$PWD/designerview.cpp \
           $$PWD/qmlcodeeditorwidget.cpp \
           $$PWD/buildswidget.cpp \
           $$PWD/projectoptionswidget.cpp \
           $$PWD/controlcreationmanager.cpp \
           $$PWD/helpwidget.cpp \
           $$PWD/webenginehelpviewer.cpp \
           $$PWD/helpviewer.cpp \
           $$PWD/form.cpp \
           $$PWD/themechooserwidget.cpp \
           $$PWD/scalingwidget.cpp \
           $$PWD/projecttemplateswidget.cpp \
           $$PWD/rowbar.cpp \
           $$PWD/textutils.cpp \
           $$PWD/linenumberband.cpp \
           $$PWD/breakpointband.cpp \
           $$PWD/bracketband.cpp \
           $$PWD/qmlcodedocument.cpp \
           $$PWD/documentmanager.cpp \
           $$PWD/markband.cpp \
           $$PWD/toolbar.cpp \
           $$PWD/toolbutton.cpp \
           $$PWD/controlremovingmanager.cpp \
           $$PWD/projectexposingmanager.cpp \
           $$PWD/bootsettings.cpp \
           $$PWD/previewerserver.cpp \
           $$PWD/controlpreviewingmanager.cpp \
           $$PWD/commanddispatcher.cpp \
           $$PWD/controlpropertymanager.cpp \
           $$PWD/transparentstyle.cpp \
           $$PWD/applicationstyle.cpp \
           $$PWD/zipper.cpp \
    globalresourcespane.cpp

FORMS += $$PWD/androidcreatekeystorecertificate.ui \
         $$PWD/toolboxsettingswindow.ui

### Utils Settings
INCLUDEPATH += $$PWD/../utils
INCLUDEPATH += $$PWD/../utils/components
INCLUDEPATH += $$PWD/../utils/qmlparser
INCLUDEPATH += $$PWD/../utils/qmlparser/utils
RESOURCES   += $$PWD/../utils/resources/shared/moduleresolver/moduleresolver.qrc
LIBS        += -L$$OUT_PWD/../utils -lutils
windows:CONFIG(release, debug | release):LIBS += -L$$OUT_PWD/../utils/release
windows:CONFIG(debug, debug | release):LIBS += -L$$OUT_PWD/../utils/debug

### Defines
DEFINES += QT_QML_DEBUG_NO_WARNING \
           APP_VER='"\\\"$$VERSION\\\""' \
           APP_NAME='"\\\"$$TARGET\\\""' \
           APP_CORP='"\\\"Objectwheel, Inc.\\\""' \
           APP_DOMAIN='"\\\"objectwheel.com\\\""' \
           APP_WSSSERVER='"\\\"wss://api.objectwheel.com:4443\\\""' \
           APP_HTTPSSERVER='"\\\"http://api.objectwheel.com:8080\\\""' \
           APP_GITVER='"\\\"$$GIT_VERSION\\\""' \
           APP_GITHASH='"\\\"$$GIT_HASH\\\""' \
           APP_GITDATE='"\\\"$$GIT_DATE\\\""'

### Installs
macx {
    interpreter.files = $$OUT_PWD/../interpreter/interpreter
    interpreter.path = Contents/MacOS
    previewer.files = $$OUT_PWD/../previewer/previewer
    previewer.path = Contents/MacOS
    themer.files = $$OUT_PWD/../themer/themer
    themer.path = Contents/MacOS
    utils.files = $$OUT_PWD/../utils/libutils.dylib
    utils.path = Contents/Frameworks
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Objectwheel -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/interpreter -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/previewer -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/themer -change libutils.dylib @loader_path/../Frameworks/libutils.dylib)
    docs.files = $$PWD/resources/docs
    docs.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += interpreter previewer themer utils docs
} else:unix {
    interpreter.files = $$OUT_PWD/../interpreter/interpreter
    interpreter.path = $$OUT_PWD/
    previewer.files = $$OUT_PWD/../previewer/previewer
    previewer.path = $$OUT_PWD/
    themer.files = $$OUT_PWD/../themer/themer
    themer.path = $$OUT_PWD/
    utils.files = $$OUT_PWD/../utils/libutils.so
    utils.path = $$OUT_PWD/
    docs.files = $$PWD/resources/docs/*
    docs.path = $$OUT_PWD/docs
    INSTALLS += interpreter previewer themer utils docs
} else:windows {
    CONFIG(debug, debug | release):COMPILING_MODE = debug
    CONFIG(release, debug | release):COMPILING_MODE = release

    FILES_TO_COPY = $$OUT_PWD/../interpreter/$$COMPILING_MODE/interpreter.exe
    FILES_TO_COPY += $$OUT_PWD/../previewer/$$COMPILING_MODE/previewer.exe
    FILES_TO_COPY += $$OUT_PWD/../themer/$$COMPILING_MODE/themer.exe
    FILES_TO_COPY += $$OUT_PWD/../utils/$$COMPILING_MODE/utils.dll
    DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$COMPILING_MODE))

    for (FILE, FILES_TO_COPY) {
        FILE_PATH = $$shell_quote($$shell_path($$FILE))
        QMAKE_POST_LINK += $$QMAKE_COPY $$FILE_PATH $$DESTINATION_DIR $$escape_expand(\\n)
    }

    DIRS_TO_COPY = $$PWD/resources/docs
    DESTINATION_DIR = $$shell_quote($$shell_path($$OUT_PWD/$$COMPILING_MODE/docs))

    for (DIR, DIRS_TO_COPY) {
        DIR_PATH = $$shell_quote($$shell_path($$DIR))
        QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$DIR_PATH $$DESTINATION_DIR $$escape_expand(\\n)
    }
}
