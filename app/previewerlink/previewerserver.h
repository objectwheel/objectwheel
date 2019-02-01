#ifndef PREVIEWERSOCKET_H
#define PREVIEWERSOCKET_H

#include <previewercommands.h>

#include <QPointer>
#include <QLocalSocket>

class QTimer;
class QLocalServer;

class PreviewerServer final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PreviewerServer)

public:
    explicit PreviewerServer(QObject* parent = nullptr);
    QString serverName() const;
    bool isConnected() const;

public slots:
    void abort();
    void close();
    void listen(const QString& serverName);
    void send(const PreviewerCommands& command, const QByteArray& data = QByteArray());

signals:
    void connected();
    void disconnected();
    void connectionTimeout();
    void dataArrived(const PreviewerCommands& command, const QByteArray& data);

private slots:
    void onNewConnection();
    void onReadReady();
    void onError(QLocalSocket::LocalSocketError socketError);

private:
    void send(QLocalSocket* socket, const PreviewerCommands& command, const QByteArray& data = QByteArray());

private:
    quint32 m_blockSize;
    QLocalServer* m_server;
    QTimer* m_checkAliveTimer;
    QPointer<QLocalSocket> m_socket;
};

#endif // PREVIEWERSOCKET_H