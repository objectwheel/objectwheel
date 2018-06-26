#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>

#include <previewercommands.h>

class CommandDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit CommandDispatcher(QObject* parent = nullptr);

public slots:
    void onDataReceived(PreviewerCommands command, QDataStream& dataStream);

signals:
    void terminate();
    void init();
};

#endif // COMMANDDISPATCHER_H