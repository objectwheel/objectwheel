HEADERS += \
    $$PWD/fontsizespinbox.h \
    $$PWD/filewidget.h \
    $$PWD/contextpanewidgetrectangle.h \
    $$PWD/contextpanewidgetimage.h \
    $$PWD/contextpanewidget.h \
    $$PWD/contextpanetextwidget.h \
    $$PWD/colorbutton.h \
    $$PWD/colorbox.h \
    $$PWD/customcolordialog.h \
    $$PWD/gradientline.h \
    $$PWD/huecontrol.h \
    $$PWD/qmleditorwidgets_global.h

SOURCES += \
    $$PWD/fontsizespinbox.cpp \
    $$PWD/filewidget.cpp \
    $$PWD/contextpanewidgetrectangle.cpp \
    $$PWD/contextpanewidgetimage.cpp \
    $$PWD/contextpanewidget.cpp \
    $$PWD/contextpanetextwidget.cpp \
    $$PWD/colorbox.cpp \
    $$PWD/customcolordialog.cpp \
    $$PWD/huecontrol.cpp \
    $$PWD/gradientline.cpp \
    $$PWD/colorbutton.cpp

FORMS += \
    $$PWD/contextpanewidgetrectangle.ui \
    $$PWD/contextpanewidgetimage.ui \
    $$PWD/contextpanewidgetborderimage.ui \
    $$PWD/contextpanetext.ui

include($$PWD/easingpane/easingpane.pri)