#include <renderserver.h>
#include <serializeenum.h>

#include <QTimer>
#include <QLocalServer>

RenderServer::RenderServer(QObject* parent)
    : QObject(parent)
    , m_blockSize(0)
    , m_server(new QLocalServer(this))
    , m_checkAliveTimer(new QTimer(this))
{
    m_checkAliveTimer->setInterval(4000);

    connect(m_checkAliveTimer, &QTimer::timeout, this, &RenderServer::connectionTimeout);

    connect(m_server, &QLocalServer::newConnection, this, &RenderServer::onNewConnection);
}

QString RenderServer::serverName() const
{
    return m_server->serverName();
}

bool RenderServer::isConnected() const
{
    return !m_socket.isNull();
}

// Only aborts current connection, server remains active
void RenderServer::abort()
{
    m_checkAliveTimer->stop();
    if (m_socket)
        m_socket->abort();
    m_socket.clear();
}

// Existing connections aren't effected, only server deactivated
void RenderServer::close()
{
    m_server->close();
}

void RenderServer::listen(const QString& serverName)
{
    m_server->listen(serverName);
}

void RenderServer::send(const RendererCommands& command, const QByteArray& data)
{
    send(m_socket, command, data);
}

void RenderServer::send(QLocalSocket* socket, const RendererCommands& command, const QByteArray& data)
{
    if (!socket || !socket->isOpen() || !socket->isWritable())
        return;

    QByteArray block;
    {
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);
        out << quint32(0);
        out << command;
        out << data;
        out.device()->seek(0);
        out << quint32(block.size() - sizeof(quint32));
    }
    socket->write(block);
    socket->flush();
}

void RenderServer::onNewConnection()
{
    QLocalSocket* pendingConnection = m_server->nextPendingConnection();

    if (m_socket.isNull()) {
        m_blockSize = 0;
        m_socket = pendingConnection;

        m_checkAliveTimer->start();

        connect(m_socket, &QLocalSocket::disconnected, m_checkAliveTimer, &QTimer::stop);
        connect(m_socket, &QLocalSocket::disconnected, this, &RenderServer::disconnected);
        connect(m_socket, &QLocalSocket::disconnected, m_socket, &QLocalSocket::deleteLater);
        connect(m_socket, &QLocalSocket::readyRead, this, &RenderServer::onReadReady);
        connect(m_socket, qOverload<QLocalSocket::LocalSocketError>(&QLocalSocket::error),
                this, &RenderServer::onError);

        emit connected();
    } else {
        send(pendingConnection, RendererCommands::Terminate);
        pendingConnection->abort();
    }
}

void RenderServer::onReadReady()
{
    QDataStream incoming(m_socket);
    incoming.setVersion(QDataStream::Qt_5_12);

    if (m_blockSize == 0) {
        if (m_socket->bytesAvailable() < (int)sizeof(quint32))
            return;

        incoming >> m_blockSize;
    }

    if (m_socket->bytesAvailable() < m_blockSize || incoming.atEnd())
        return;

    QByteArray data;
    RendererCommands command;

    incoming >> command;
    incoming >> data;

    m_blockSize = 0;

    if (command == RendererCommands::ConnectionAlive)
        m_checkAliveTimer->start();
    else
        emit dataArrived(command, data);

    if (m_socket->bytesAvailable() >= (int)sizeof(quint32))
        onReadReady();
}

void RenderServer::onError(QLocalSocket::LocalSocketError socketError)
{
    qWarning() << socketError << "Socket error, in" << __FILE__ << ":" << __LINE__;
}

