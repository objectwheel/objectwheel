#include <components.h>
#include <QQmlEngine>

#ifdef OW_APIAI
#include <apiai.h>
#endif

#ifdef OW_AUDIORECORDER
#include <audiorecorder.h>
#endif

#ifdef OW_AUDIOPLAYER
#include <audioplayer.h>
#endif

#ifdef OW_AISPEAK
#include <aispeak.h>
#endif

#ifdef OW_FM
#include <filemanager.h>
#endif

#ifdef OW_FIREBASE_DATABASE
#include <firebasedatabase.h>
#endif

void Components::init()
{
#ifdef OW_APIAI
    qmlRegisterType<ApiAi>("Objectwheel.Ai", 1, 0, "ApiAi");
#endif

#ifdef OW_AISPEAK
    qmlRegisterType<AiSpeak>("Objectwheel.Ai", 1, 0, "AiSpeak");
#endif

#ifdef OW_AUDIORECORDER
    qmlRegisterType<AudioRecorder>("Objectwheel.Multimedia", 1, 0, "AudioRecorder");
#endif

#ifdef OW_AUDIOPLAYER
    qmlRegisterType<AudioPlayer>("Objectwheel.Multimedia", 1, 0, "AudioPlayer");
#endif

#ifdef OW_FM
    qmlRegisterType<FileManager>("Objectwheel.IO", 1, 0, "FileManager");
#endif

#ifdef OW_FIREBASE_DATABASE
    qmlRegisterType<FirebaseDatabase>("Objectwheel.Firebase", 1, 0, "FirebaseDatabase");
#endif
}
