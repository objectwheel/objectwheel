INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

FORMS += \
         $$PWD/androidcreatekeystorecertificate.ui \
    $$PWD/toolboxsettingswindow.ui

HEADERS += $$PWD/mainwindow.h \
           $$PWD/flatbutton.h \
           $$PWD/propertieswidget.h \
           $$PWD/css.h \
           $$PWD/scrollarea.h \
           $$PWD/usermanager.h \
           $$PWD/savemanager.h \
           $$PWD/projectmanager.h \
           $$PWD/dirlocker.h \
           $$PWD/zipper.h \
           $$PWD/toolsmanager.h \
           $$PWD/delayer.h \
           $$PWD/designmanager.h \
           $$PWD/control.h \
           $$PWD/formscene.h \
           $$PWD/qmlpreviewer.h \
           $$PWD/savetransaction.h \
           $$PWD/parserworker.h \
           $$PWD/parsercontroller.h \
           $$PWD/controlscene.h \
           $$PWD/controlview.h \
           $$PWD/formview.h \
           $$PWD/loadingindicator.h \
           $$PWD/qmleditorview.h \
           $$PWD/qmlcodeeditor.h \
           $$PWD/qmlhighlighter.h \
           $$PWD/completionhelper.h \
           $$PWD/fileexplorer.h \
           $$PWD/filelist.h \
           $$PWD/qmlformatter.h \
           $$PWD/toolbox.h \
           $$PWD/toolboxtree.h \
           $$PWD/filterlineedit.h \
           $$PWD/view.h \
           $$PWD/progresswidget.h \
           $$PWD/global.h \
           $$PWD/executivewidget.h \
           $$PWD/issuesbox.h \
           $$PWD/consolebox.h \
           $$PWD/searchbox.h \
           $$PWD/outputwidget.h \
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
           $$PWD/projectswidget.h \
           $$PWD/loginwidget.h \
           $$PWD/aboutwindow.h \
           $$PWD/buildswindow.h \
           $$PWD/preferenceswindow.h \
    $$PWD/toolboxsettingswindow.h \
    $$PWD/menumanager.h \
    $$PWD/inspectorpane.h \
    $$PWD/formspane.h

SOURCES += $$PWD/main.cpp\
           $$PWD/mainwindow.cpp \
           $$PWD/flatbutton.cpp \
           $$PWD/propertieswidget.cpp \
           $$PWD/css.cpp \
           $$PWD/scrollarea.cpp \
           $$PWD/usermanager.cpp \
           $$PWD/savemanager.cpp \
           $$PWD/projectmanager.cpp \
           $$PWD/dirlocker.cpp \
           $$PWD/zipper.cpp \
           $$PWD/toolsmanager.cpp \
           $$PWD/delayer.cpp \
           $$PWD/designmanager.cpp \
           $$PWD/control.cpp \
           $$PWD/formscene.cpp \
           $$PWD/qmlpreviewer.cpp \
           $$PWD/savetransaction.cpp \
           $$PWD/parserworker.cpp \
           $$PWD/parsercontroller.cpp \
           $$PWD/controlscene.cpp \
           $$PWD/controlview.cpp \
           $$PWD/formview.cpp \
           $$PWD/loadingindicator.cpp \
           $$PWD/qmleditorview.cpp \
           $$PWD/qmlcodeeditor.cpp \
           $$PWD/qmlhighlighter.cpp \
           $$PWD/completionhelper.cpp \
           $$PWD/fileexplorer.cpp \
           $$PWD/filelist.cpp \
           $$PWD/qmlformatter.cpp \
           $$PWD/toolbox.cpp \
           $$PWD/toolboxtree.cpp \
           $$PWD/filterlineedit.cpp \
           $$PWD/view.cpp \
           $$PWD/progresswidget.cpp \
           $$PWD/executivewidget.cpp \
           $$PWD/issuesbox.cpp \
           $$PWD/consolebox.cpp \
           $$PWD/searchbox.cpp \
           $$PWD/outputwidget.cpp \
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
           $$PWD/projectswidget.cpp \
           $$PWD/loginwidget.cpp \
           $$PWD/aboutwindow.cpp \
           $$PWD/buildswindow.cpp \
           $$PWD/preferenceswindow.cpp \
    $$PWD/toolboxsettingswindow.cpp \
    $$PWD/menumanager.cpp \
    $$PWD/inspectorpane.cpp \
    $$PWD/formspane.cpp

include($$PWD/platform/platform.pri)
include($$PWD/resources/resources.pri)
include($$PWD/components/components.pri)
