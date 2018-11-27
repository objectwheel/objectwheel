#include <components.h>
#include <globalresources.h>
#include <offlinestorage.h>
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

namespace Components {

void init()
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

#ifdef OW_FIREBASE_DATABASE
    qmlRegisterType<FirebaseDatabase>("Objectwheel.Database", 1, 0, "FirebaseDatabase");
#endif

#ifdef OW_FM
    qmlRegisterSingletonType<OfflineStorage>("Objectwheel.Core", 1, 0, "FileManager",
                                             [] (QQmlEngine* engine, QJSEngine* jsEngine) -> QObject* {
        Q_UNUSED(jsEngine)
        return new FileManager(engine);
    });
#endif

    qmlRegisterSingletonType<GlobalResources>("Objectwheel.GlobalResources", 1, 0, "GlobalResources",
                                              [] (QQmlEngine* engine, QJSEngine* jsEngine) -> QObject* {
        Q_UNUSED(engine)
        Q_UNUSED(jsEngine)
        return GlobalResources::instance();
    });

    qmlRegisterSingletonType<OfflineStorage>("Objectwheel.Core", 1, 0, "OfflineStorage",
                                             [] (QQmlEngine* engine, QJSEngine* jsEngine) -> QObject* {
        Q_UNUSED(jsEngine)
        return new OfflineStorage(engine);
    });
}
}