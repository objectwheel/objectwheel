INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

HEADERS += $$PWD/components.h \
           $$PWD/apiai.h \
           $$PWD/audiorecorder.h \
           $$PWD/audioplayer.h \
           $$PWD/aispeak.h \
           $$PWD/firebasedatabase.h

SOURCES += $$PWD/components.cpp \
           $$PWD/apiai.cpp \
           $$PWD/audiorecorder.cpp \
           $$PWD/audioplayer.cpp \
           $$PWD/aispeak.cpp \
           $$PWD/firebasedatabase.cpp

DEFINES += OW_APIAI \
           OW_AUDIORECORDER \
           OW_AUDIOPLAYER \
           OW_AISPEAK \
           OW_FM \
           OW_FIREBASE_DATABASE