#ifndef PREVIEWERSOCKET_H
#define PREVIEWERSOCKET_H

#include <previewercommands.h>

#include <QLocalSocket>

class QTimer;

class PreviewerSocket : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PreviewerSocket)

    using QObject::connect;

public:
    explicit PreviewerSocket(QObject* parent = nullptr);
    QLocalSocket::LocalSocketState state();

public slots:
    void abort();
    void stop();
    void start(const QString& serverName);
    void send(const PreviewerCommands& command, const QByteArray& data = QByteArray());

signals:
    void disconnected();
    void dataArrived(const PreviewerCommands& command, const QByteArray& data);

private slots:
    void connect();
    void sendAlive();
    void onReadReady();
    void onError(QLocalSocket::LocalSocketError socketError);

private:
    quint32 m_blockSize;
    QString m_serverName;
    QLocalSocket* m_socket;
    QTimer* m_reconnectionTimer;
    QTimer* m_sendAliveTimer;
};

#endif // PREVIEWERSOCKET_H