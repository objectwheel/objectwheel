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
							 $$PWD/bubblehead.h \
							 $$PWD/qmleditor.h \
							 $$PWD/pageswidget.h \
							 $$PWD/splashscreen.h \
							 $$PWD/splashscreen_p.h \
    $$PWD/usermanager.h \
    $$PWD/savemanager.h \
    $$PWD/projectmanager.h \
    $$PWD/dirlocker.h \
    $$PWD/zipper.h \
    $$PWD/toolsmanager.h \
    $$PWD/scenemanager.h \
    $$PWD/projectsscreen.h

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
							 $$PWD/bubblehead.cpp \
							 $$PWD/qmleditor.cpp \
							 $$PWD/pageswidget.cpp \
							 $$PWD/splashscreen.cpp \
							 $$PWD/splashscreen_p.cpp \
    $$PWD/usermanager.cpp \
    $$PWD/savemanager.cpp \
    $$PWD/projectmanager.cpp \
    $$PWD/dirlocker.cpp \
    $$PWD/zipper.cpp \
    $$PWD/toolsmanager.cpp \
    $$PWD/scenemanager.cpp \
    $$PWD/projectsscreen.cpp

RESOURCES                 += $$PWD/resources.qrc

DISTFILES                 += android/AndroidManifest.xml \
                             android/res/values/libs.xml \
                             android/build.gradle \
                             android/res/drawable-mdpi/icon.png \
                             android/res/drawable-ldpi/icon.png \
                             android/res/drawable-hdpi/icon.png

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

include($$PWD/ticks/ticks.pri)
include($$PWD/components/components.pri)
include($$PWD/platform_spesific/ios/ios.pri)
