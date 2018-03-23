#include <components.h>

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
#include <fm.h>
#endif

void Components::init()
{
    #ifdef OW_APIAI
    ApiAi::registerQmlType();
    #endif

    #ifdef OW_AUDIORECORDER
    AudioRecorder::registerQmlType();
    #endif

    #ifdef OW_AUDIOPLAYER
    AudioPlayer::registerQmlType();
    #endif

    #ifdef OW_AISPEAK
    AiSpeak::registerQmlType();
    #endif

    #ifdef OW_FM
    FM::registerQmlType();
    #endif
}
