#include <commanddispatcher.h>

CommandDispatcher::CommandDispatcher(QObject *parent) : QObject(parent)
{

}

void CommandDispatcher::onDataReceived(PreviewerCommands command, QDataStream& dataStream)
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
