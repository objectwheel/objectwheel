#include <components.h>

void Components::init()
{
	ApiAi::registerQmlType();
	AudioRecorder::registerQmlType();
	AudioPlayer::registerQmlType();
	AiSpeak::registerQmlType();
	DocumentHandler::registerQmlType();
	FileManager::registerQmlType();
	QQuickFolderListModel::registerQmlType();
}
