#include <commanddispatcher.h>

CommandDispatcher::CommandDispatcher(QObject *parent) : QObject(parent)
{

}

void CommandDispatcher::onDataReceived(const PreviewerCommands& command, const QByteArray& data)
{
    switch (command) {
    case Terminate:
        emit terminate();
        break;

    case Init:
        emit init();
        break;

    default:
        break;
    }
}
