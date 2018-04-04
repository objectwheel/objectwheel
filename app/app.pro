### App Settings
TEMPLATE = app
TARGET   = Objectwheel
#DEFINES += PREVIEWER_DEBUG

QT += quickwidgets qml quick help
QT += widgets network websockets svg concurrent quickcontrols2
QT += webenginewidgets webchannel webengine webenginecore

### Includes
include($$PWD/version.pri)
include($$PWD/aes/aes.pri)
include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)
include($$PWD/../shared/resources/shared_resources.pri)

### Sources
INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

HEADERS += $$PWD/mainwindow.h \
           $$PWD/flatbutton.h \
           $$PWD/css.h \
           $$PWD/scrollarea.h \
           $$PWD/dirlocker.h \
           $$PWD/control.h \
           $$PWD/savetransaction.h \
           $$PWD/qmlcodeeditor.h \
           $$PWD/qmlhighlighter.h \
           $$PWD/completionhelper.h \
           $$PWD/fileexplorer.h \
           $$PWD/filelist.h \
           $$PWD/toolboxtree.h \
           $$PWD/view.h \
           $$PWD/global.h \
           $$PWD/issuesbox.h \
           $$PWD/consolebox.h \
           $$PWD/searchbox.h \
           $$PWD/controlwatcher.h \
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
           $$PWD/backendmanager.h \
           $$PWD/savebackend.h \
           $$PWD/projectbackend.h \
           $$PWD/toolsbackend.h \
           $$PWD/userbackend.h \
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
           $$PWD/interpreterbackend.h \
           $$PWD/previewerbackend.h \
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
           $$PWD/exposerbackend.h \
           $$PWD/helpwidget.h \
           $$PWD/webenginehelpviewer.h \
           $$PWD/helpviewer.h \
           $$PWD/hashfactory.h \
           $$PWD/form.h \
           $$PWD/themechooserwidget.h

SOURCES += $$PWD/main.cpp\
           $$PWD/mainwindow.cpp \
           $$PWD/flatbutton.cpp \
           $$PWD/css.cpp \
           $$PWD/scrollarea.cpp \
           $$PWD/dirlocker.cpp \
           $$PWD/control.cpp \
           $$PWD/savetransaction.cpp \
           $$PWD/qmlcodeeditor.cpp \
           $$PWD/qmlhighlighter.cpp \
           $$PWD/completionhelper.cpp \
           $$PWD/fileexplorer.cpp \
           $$PWD/filelist.cpp \
           $$PWD/toolboxtree.cpp \
           $$PWD/view.cpp \
           $$PWD/issuesbox.cpp \
           $$PWD/consolebox.cpp \
           $$PWD/searchbox.cpp \
           $$PWD/controlwatcher.cpp \
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
           $$PWD/backendmanager.cpp \
           $$PWD/savebackend.cpp \
           $$PWD/projectbackend.cpp \
           $$PWD/toolsbackend.cpp \
           $$PWD/userbackend.cpp \
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
           $$PWD/interpreterbackend.cpp \
           $$PWD/previewerbackend.cpp \
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
           $$PWD/exposerbackend.cpp \
           $$PWD/helpwidget.cpp \
           $$PWD/webenginehelpviewer.cpp \
           $$PWD/helpviewer.cpp \
           $$PWD/hashfactory.cpp \
           $$PWD/form.cpp \
           $$PWD/themechooserwidget.cpp

FORMS += $$PWD/androidcreatekeystorecertificate.ui \
         $$PWD/toolboxsettingswindow.ui

### Library Settings
INCLUDEPATH += $$PWD/../shared
INCLUDEPATH += $$PWD/../lib
INCLUDEPATH += $$PWD/../lib/fit
INCLUDEPATH += $$PWD/../lib/miniz
LIBS        += -L$$OUT_PWD/../lib -lobjectwheel

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
    interpreter.files = $$OUT_PWD/../objectwheel-interpreter/objectwheel-interpreter
    interpreter.path = Contents/MacOS
    previewer.files = $$OUT_PWD/../objectwheel-previewer/objectwheel-previewer
    previewer.path = Contents/MacOS
    themer.files = $$OUT_PWD/../objectwheel-themer/objectwheel-themer
    themer.path = Contents/MacOS
    lib.files = $$OUT_PWD/../lib/libobjectwheel.dylib
    lib.path = Contents/Frameworks
    QMAKE_POST_LINK += $$system(install_name_tool $$OUT_PWD/Objectwheel.app/Contents/MacOS/Objectwheel -change libobjectwheel.dylib @loader_path/../Frameworks/libobjectwheel.dylib)
    docs.files = $$PWD/resources/docs
    docs.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += interpreter previewer themer lib docs
} else {
    interpreter.files = $$OUT_PWD/../objectwheel-interpreter/objectwheel-interpreter
    interpreter.path = $$OUT_PWD/
    previewer.files = $$OUT_PWD/../objectwheel-previewer/objectwheel-previewer
    previewer.path = $$OUT_PWD/
    themer.files = $$OUT_PWD/../objectwheel-themer/objectwheel-themer
    themer.path = $$OUT_PWD/
    windows {
        lib.files = $$OUT_PWD/../lib/libobjectwheel.dll
    }
    linux {
        lib.files = $$OUT_PWD/../lib/libobjectwheel.so
    }
    lib.path = $$OUT_PWD/
    docs.files = $$PWD/resources/docs/*
    docs.path = $$OUT_PWD/docs
    QMAKE_EXTRA_TARGETS += interpreter previewer themer lib docs
}