INCLUDEPATH               += $$PWD

HEADERS                   += $$PWD/mainwindow.h \
                             $$PWD/listwidget.h

SOURCES                   += $$PWD/main.cpp\
                             $$PWD/mainwindow.cpp \
                             $$PWD/listwidget.cpp

FORMS                     += $$PWD/mainwindow.ui

RESOURCES                 += $$PWD/resources.qrc

DISTFILES                 += android/AndroidManifest.xml \
							 android/res/values/libs.xml \
							 android/build.gradle

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

include($$PWD/ticks/ticks.pri)
