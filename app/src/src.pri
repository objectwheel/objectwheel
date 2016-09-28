INCLUDEPATH               += $$PWD

HEADERS                   += $$PWD/mainwindow.h \
                             $$PWD/listwidget.h

SOURCES                   += $$PWD/main.cpp\
                             $$PWD/mainwindow.cpp \
                             $$PWD/listwidget.cpp

FORMS                     += $$PWD/mainwindow.ui

RESOURCES                 += $$PWD/resources.qrc

DISTFILES                 += $$PWD/android/AndroidManifest.xml \
                             $$PWD/android/res/values/libs.xml \
                             $$PWD/android/build.gradle \
                             $$PWD/images/resize-icon.png

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

include($$PWD/ticks/ticks.pri)