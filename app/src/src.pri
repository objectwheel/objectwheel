INCLUDEPATH               += $$PWD

HEADERS                   += $$PWD/mainwindow.h \
                             $$PWD/mainwindow_p.h \
                             $$PWD/listwidget.h \
                             $$PWD/covermenu.h \
                             $$PWD/titlebar.h \
                             $$PWD/flatbutton.h \
                             $$PWD/propertieswidget.h \
                             $$PWD/propertyitem.h \
                             $$PWD/css.h \
                             $$PWD/switch.h \
                             $$PWD/container.h \
                             $$PWD/bindingwidget.h \
                             $$PWD/lineedit.h \
                             $$PWD/combobox.h \
                             $$PWD/scrollarea.h \
                             $$PWD/about.h \
                             $$PWD/qmleditor.h \
                             $$PWD/splashscreen.h \
                             $$PWD/splashscreen_p.h \
                             $$PWD/usermanager.h \
                             $$PWD/savemanager.h \
                             $$PWD/projectmanager.h \
                             $$PWD/dirlocker.h \
                             $$PWD/zipper.h \
                             $$PWD/toolsmanager.h \
                             $$PWD/scenemanager.h \
                             $$PWD/projectsscreen.h \
                             $$PWD/loginscreen.h \
                             $$PWD/buildsscreen.h \
                             $$PWD/eventswidget.h \
                             $$PWD/delayer.h \
                             $$PWD/designmanager.h \
                             $$PWD/control.h \
                             $$PWD/windowscene.h \
                             $$PWD/qmlpreviewer.h \
                             $$PWD/controltransaction.h \
                             $$PWD/parserworker.h \
                             $$PWD/parsercontroller.h \
                             $$PWD/windowview.h \
                             $$PWD/windowswidget.h

SOURCES                   += $$PWD/main.cpp\
                             $$PWD/mainwindow.cpp \
                             $$PWD/listwidget.cpp \
                             $$PWD/covermenu.cpp \
                             $$PWD/titlebar.cpp \
                             $$PWD/flatbutton.cpp \
                             $$PWD/propertieswidget.cpp \
                             $$PWD/propertyitem.cpp \
                             $$PWD/css.cpp \
                             $$PWD/switch.cpp \
                             $$PWD/container.cpp \
                             $$PWD/bindingwidget.cpp \
                             $$PWD/lineedit.cpp \
                             $$PWD/combobox.cpp \
                             $$PWD/scrollarea.cpp \
                             $$PWD/about.cpp \
                             $$PWD/qmleditor.cpp \
                             $$PWD/splashscreen.cpp \
                             $$PWD/splashscreen_p.cpp \
                             $$PWD/usermanager.cpp \
                             $$PWD/savemanager.cpp \
                             $$PWD/projectmanager.cpp \
                             $$PWD/dirlocker.cpp \
                             $$PWD/zipper.cpp \
                             $$PWD/toolsmanager.cpp \
                             $$PWD/scenemanager.cpp \
                             $$PWD/projectsscreen.cpp \
                             $$PWD/loginscreen.cpp \
                             $$PWD/buildsscreen.cpp \
                             $$PWD/eventswidget.cpp \
                             $$PWD/delayer.cpp \
                             $$PWD/designmanager.cpp \
                             $$PWD/control.cpp \
                             $$PWD/windowscene.cpp \
                             $$PWD/qmlpreviewer.cpp \
                             $$PWD/controltransaction.cpp \
                             $$PWD/parserworker.cpp \
                             $$PWD/parsercontroller.cpp \
                             $$PWD/windowview.cpp \
                             $$PWD/windowswidget.cpp

RESOURCES                 += $$PWD/resources.qrc

DISTFILES                 += android/AndroidManifest.xml \
                             android/res/values/libs.xml \
                             android/build.gradle \
                             android/res/drawable-mdpi/icon.png \
                             android/res/drawable-ldpi/icon.png \
                             android/res/drawable-hdpi/icon.png

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

include($$PWD/components/components.pri)
include($$PWD/platform_spesific/ios/ios.pri)
