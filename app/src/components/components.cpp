#include <components.h>

void Components::init()
{
	ApiAi::registerQmlType();
	AudioRecorder::registerQmlType();
	AudioPlayer::registerQmlType();
}
