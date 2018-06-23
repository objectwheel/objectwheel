#ifndef PREVIEWERSOCKET_H
#define PREVIEWERSOCKET_H

#include <previewercommands.h>

#include <QPointer>

class QTimer;
class QLocalServer;
class QLocalSocket;

class PreviewerServer final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PreviewerServer)

public:
    explicit PreviewerServer(QObject* parent = nullptr);

public slots:
    void close();
    void listen(const QString& serverName);
    void send(PreviewerCommands command, const QByteArray& data = QByteArray());

signals:
    void disconnected();
    void connectionTimeout();
    void dataArrived(PreviewerCommands command, QDataStream& dataStream);

private slots:
    void onNewConnection();
    void onReadReady();

private:
    void send(QLocalSocket* socket, PreviewerCommands command, const QByteArray& data = QByteArray());

private:
    int m_blockSize;
    QLocalServer* m_server;
    QTimer* m_checkAliveTimer;
    QPointer<QLocalSocket> m_socket;
};

#endif // PREVIEWERSOCKET_H