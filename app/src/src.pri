INCLUDEPATH               += $$PWD

HEADERS                   += $$PWD/mainwindow.h \
                             $$PWD/flatbutton.h \
                             $$PWD/propertieswidget.h \
                             $$PWD/css.h \
                             $$PWD/scrollarea.h \
                             $$PWD/about.h \
                             $$PWD/usermanager.h \
                             $$PWD/savemanager.h \
                             $$PWD/projectmanager.h \
                             $$PWD/dirlocker.h \
                             $$PWD/zipper.h \
                             $$PWD/toolsmanager.h \
                             $$PWD/projectsscreen.h \
                             $$PWD/loginscreen.h \
                             $$PWD/buildsscreen.h \
                             $$PWD/delayer.h \
                             $$PWD/designmanager.h \
                             $$PWD/control.h \
                             $$PWD/formscene.h \
                             $$PWD/qmlpreviewer.h \
                             $$PWD/controltransaction.h \
                             $$PWD/parserworker.h \
                             $$PWD/parsercontroller.h \
                             $$PWD/controlscene.h \
                             $$PWD/controlview.h \
                             $$PWD/controlsscrollpanel.h \
                             $$PWD/formview.h \
                             $$PWD/formswidget.h \
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
                             $$PWD/centralwidget.h \
                             $$PWD/progresswidget.h \
                             $$PWD/global.h \
    $$PWD/executivewidget.h

SOURCES                   += $$PWD/main.cpp\
                             $$PWD/mainwindow.cpp \
                             $$PWD/flatbutton.cpp \
                             $$PWD/propertieswidget.cpp \
                             $$PWD/css.cpp \
                             $$PWD/scrollarea.cpp \
                             $$PWD/about.cpp \
                             $$PWD/usermanager.cpp \
                             $$PWD/savemanager.cpp \
                             $$PWD/projectmanager.cpp \
                             $$PWD/dirlocker.cpp \
                             $$PWD/zipper.cpp \
                             $$PWD/toolsmanager.cpp \
                             $$PWD/projectsscreen.cpp \
                             $$PWD/loginscreen.cpp \
                             $$PWD/buildsscreen.cpp \
                             $$PWD/delayer.cpp \
                             $$PWD/designmanager.cpp \
                             $$PWD/control.cpp \
                             $$PWD/formscene.cpp \
                             $$PWD/qmlpreviewer.cpp \
                             $$PWD/controltransaction.cpp \
                             $$PWD/parserworker.cpp \
                             $$PWD/parsercontroller.cpp \
                             $$PWD/controlscene.cpp \
                             $$PWD/controlview.cpp \
                             $$PWD/controlsscrollpanel.cpp \
                             $$PWD/formview.cpp \
                             $$PWD/formswidget.cpp \
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
                             $$PWD/centralwidget.cpp \
                             $$PWD/progresswidget.cpp \
    $$PWD/executivewidget.cpp

RESOURCES                 += $$PWD/resources.qrc

DISTFILES                 += android/AndroidManifest.xml \
                             android/res/values/libs.xml \
                             android/build.gradle \
                             android/res/drawable-mdpi/icon.png \
                             android/res/drawable-ldpi/icon.png \
                             android/res/drawable-hdpi/icon.png

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

include($$PWD/components/components.pri)
