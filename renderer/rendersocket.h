#ifndef RENDERSOCKET_H
#define RENDERSOCKET_H

#include <renderercommands.h>

#include <QLocalSocket>

class QTimer;

class RenderSocket final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RenderSocket)

    using QObject::connect;

public:
    explicit RenderSocket(QObject* parent = nullptr);
    QLocalSocket::LocalSocketState state();

public slots:
    void abort();
    void stop();
    void start(const QString& serverName);
    void send(const RendererCommands& command, const QByteArray& data = QByteArray());

signals:
    void disconnected();
    void dataArrived(const RendererCommands& command, const QByteArray& data);

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

#endif // RENDERSOCKET_H