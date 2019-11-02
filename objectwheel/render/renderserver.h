#ifndef RENDERSERVER_H
#define RENDERSERVER_H

#include <renderercommands.h>

#include <QPointer>
#include <QLocalSocket>

class QTimer;
class QLocalServer;

class RenderServer final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RenderServer)

public:
    explicit RenderServer(QObject* parent = nullptr);
    QString serverName() const;
    bool isConnected() const;

public slots:
    void abort();
    void close();
    void listen(const QString& serverName);
    void send(const RendererCommands& command, const QByteArray& data = QByteArray());

signals:
    void connected();
    void disconnected();
    void connectionTimeout();
    void dataArrived(const RendererCommands& command, const QByteArray& data);

private slots:
    void onNewConnection();
    void onReadReady();
    void onError(QLocalSocket::LocalSocketError socketError);

private:
    void send(QLocalSocket* socket, const RendererCommands& command, const QByteArray& data = QByteArray());

private:
    quint32 m_blockSize;
    QLocalServer* m_server;
    QTimer* m_checkAliveTimer;
    QPointer<QLocalSocket> m_socket;
};

#endif // RENDERSERVER_H