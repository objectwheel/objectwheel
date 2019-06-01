#include <components.h>
#include <offlinestorage.h>
#include <ow.h>
#include <QQmlEngine>

#ifdef OW_APIAI
#include <apiai.h>
#endif

#ifdef OW_AISPEAK
#include <aispeak.h>
#endif

#ifdef OW_AUDIORECORDER
#include <audiorecorder.h>
#endif

#ifdef OW_AUDIOPLAYER
#include <audioplayer.h>
#endif

#ifdef OW_FIREBASEDATABASE
#include <firebasedatabase.h>
#endif

#ifdef OW_TRANSLATION
#include <translation.h>
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

#ifdef OW_FIREBASEDATABASE
    qmlRegisterType<FirebaseDatabase>("Objectwheel.Database", 1, 0, "FirebaseDatabase");
#endif

#ifdef OW_TRANSLATION
    qmlRegisterSingletonType<Translation>("Objectwheel.Core", 1, 0, "Translation",
                                          [] (QQmlEngine* engine, QJSEngine* /*jsEngine*/) -> QObject* {
        return new Translation(engine);
    });
#endif

    qmlRegisterSingletonType<OfflineStorage>("Objectwheel.Core", 1, 0, "OfflineStorage",
                                             [] (QQmlEngine* engine, QJSEngine* /*jsEngine*/) -> QObject* {
        return new OfflineStorage(engine);
    });

    qmlRegisterSingletonType<Ow>("Objectwheel", 1, 0, "Ow",
                                 [] (QQmlEngine* engine, QJSEngine* /*jsEngine*/) -> QObject* {
        return new Ow(engine);
    });
}
}