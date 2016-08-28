QT          += core gui widgets qml quick quickwidgets

TARGET       = DemoApp
TEMPLATE     = app

DEPENDPATH  += $$PWD
INCLUDEPATH += $$PWD

SOURCES     += main.cpp\
			   mainwindow.cpp

HEADERS     += mainwindow.h

FORMS       += mainwindow.ui
