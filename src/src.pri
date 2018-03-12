INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)

FORMS += $$PWD/androidcreatekeystorecertificate.ui \
         $$PWD/toolboxsettingswindow.ui

HEADERS += $$PWD/mainwindow.h \
           $$PWD/flatbutton.h \
           $$PWD/css.h \
           $$PWD/scrollarea.h \
           $$PWD/dirlocker.h \
           $$PWD/zipper.h \
           $$PWD/delayer.h \
           $$PWD/control.h \
           $$PWD/formscene.h \
           $$PWD/savetransaction.h \
           $$PWD/formview.h \
           $$PWD/qmleditorview.h \
           $$PWD/qmlcodeeditor.h \
           $$PWD/qmlhighlighter.h \
           $$PWD/completionhelper.h \
           $$PWD/fileexplorer.h \
           $$PWD/filelist.h \
           $$PWD/qmlformatter.h \
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
           $$PWD/buildswindow.h \
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
           $$PWD/filemanager.h \
           $$PWD/authenticator.h \
           $$PWD/verificationwidget.h \
           $$PWD/countdown.h \
           $$PWD/robotwidget.h \
           $$PWD/forgetwidget.h \
           $$PWD/resetwidget.h \
           $$PWD/succeedwidget.h \
           $$PWD/projectdetailswidget.h \
           $$PWD/interpreterbackend.h \
           $$PWD/saveutils.h \
           $$PWD/previewresult.h \
           $$PWD/previewerbackend.h \
           $$PWD/previewercommands.h \
           $$PWD/parserutils.h \
           $$PWD/progressbar.h \
    $$PWD/centralwidget.h \
    $$PWD/designerwidget.h

SOURCES += $$PWD/main.cpp\
           $$PWD/mainwindow.cpp \
           $$PWD/flatbutton.cpp \
           $$PWD/css.cpp \
           $$PWD/scrollarea.cpp \
           $$PWD/dirlocker.cpp \
           $$PWD/zipper.cpp \
           $$PWD/delayer.cpp \
           $$PWD/control.cpp \
           $$PWD/formscene.cpp \
           $$PWD/savetransaction.cpp \
           $$PWD/formview.cpp \
           $$PWD/qmleditorview.cpp \
           $$PWD/qmlcodeeditor.cpp \
           $$PWD/qmlhighlighter.cpp \
           $$PWD/completionhelper.cpp \
           $$PWD/fileexplorer.cpp \
           $$PWD/filelist.cpp \
           $$PWD/qmlformatter.cpp \
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
           $$PWD/buildswindow.cpp \
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
           $$PWD/filemanager.cpp \
           $$PWD/authenticator.cpp \
           $$PWD/verificationwidget.cpp \
           $$PWD/countdown.cpp \
           $$PWD/robotwidget.cpp \
           $$PWD/forgetwidget.cpp \
           $$PWD/resetwidget.cpp \
           $$PWD/succeedwidget.cpp \
           $$PWD/projectdetailswidget.cpp \
           $$PWD/interpreterbackend.cpp \
           $$PWD/saveutils.cpp \
           $$PWD/previewresult.cpp \
           $$PWD/previewerbackend.cpp \
           $$PWD/parserutils.cpp \
           $$PWD/progressbar.cpp \
    $$PWD/centralwidget.cpp \
    $$PWD/designerwidget.cpp

CONFIG(release, debug | release) {
    !infile($$VERSION_FILE, PREVIOUS_VERSION, $$GIT_VERSION) {
        VERSION_DATA = PREVIOUS_VERSION=$$GIT_VERSION
        write_file($$VERSION_FILE, VERSION_DATA)
        touch($$PWD/global.h, $$VERSION_FILE)
        touch($$PWD/main.cpp, $$VERSION_FILE)
        touch($$PWD/projectswidget.cpp, $$VERSION_FILE)
    }
}