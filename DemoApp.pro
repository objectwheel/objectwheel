QT          += core gui widgets qml quick quickwidgets network

TARGET       = DemoApp
TEMPLATE     = app
CONFIG      += c++11

DEPENDPATH  += $$PWD
INCLUDEPATH += $$PWD

SOURCES     += main.cpp\
			   mainwindow.cpp \
			   listwidget.cpp

HEADERS     += mainwindow.h \
			   listwidget.h

FORMS       += mainwindow.ui

RESOURCES   += resources.qrc

include($$PWD/libs/fit/fit.pri)
include($$PWD/libs/miniz/miniz.pri)
include($$PWD/libs/ticks/ticks.pri)
