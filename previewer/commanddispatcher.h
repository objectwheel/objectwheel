#ifndef COMMANDDISPATCHER_H
#define COMMANDDISPATCHER_H

#include <QObject>

#include <previewercommands.h>

class PreviewerSocket;
struct PreviewResult;

class CommandDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit CommandDispatcher(PreviewerSocket* socket, QObject* parent = nullptr);

public slots:
    void scheduleInitializationProgress(int progress);
    void schedulePreviewDone(const QList<PreviewResult>& results);

    void onDataReceived(const PreviewerCommands& command, const QByteArray& data);

signals:
    void init();
    void terminate();
    void propertyUpdate(const QString& uid, const QString& propertyName, const QVariant& propertyValue);
    void formCreation(const QString& dir);
    void controlCreation(const QString& dir, const QString& parentUid);
    void refresh(const QString& formUid);
    void parentUpdate(const QString& newDir, const QString& uid, const QString& parentUid);
    void idUpdate(const QString& uid, const QString& newId);
    void controlDeletion(const QString& uid);
    void formDeletion(const QString& uid);
    void controlCodeUpdate(const QString& uid);
    void formCodeUpdate(const QString& uid);

private:
    PreviewerSocket* m_socket;
};

#endif // COMMANDDISPATCHER_H