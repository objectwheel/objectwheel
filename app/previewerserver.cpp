#include <previewerserver.h>

#include <QTimer>
#include <QDataStream>
#include <QLocalServer>

PreviewerServer::PreviewerServer(QObject* parent)
    : QObject(parent)
    , m_blockSize(0)
    , m_server(new QLocalServer(this))
    , m_checkAliveTimer(new QTimer(this))
{
    m_checkAliveTimer->setInterval(4000);

    connect(m_checkAliveTimer, &QTimer::timeout, this, &PreviewerServer::connectionTimeout);

    connect(m_server, &QLocalServer::newConnection, this, &PreviewerServer::onNewConnection);
}

QString PreviewerServer::serverName() const
{
    return m_server->serverName();
}

bool PreviewerServer::isConnected() const
{
    return !m_socket.isNull();
}

// Only aborts current connection, server remains active
void PreviewerServer::abort()
{
    m_checkAliveTimer->stop();
    if (m_socket)
        m_socket->abort();
    m_socket.clear();
}

// Existing connections did not effected, only server deactivated
void PreviewerServer::close()
{
    m_server->close();
}

void PreviewerServer::listen(const QString& serverName)
{
    m_server->listen(serverName);
}

void PreviewerServer::send(const PreviewerCommands& command, const QByteArray& data)
{
    send(m_socket, command, data);
}

void PreviewerServer::send(QLocalSocket* socket, const PreviewerCommands& command, const QByteArray& data)
{
    if (!socket || !socket->isOpen() || !socket->isWritable())
        return;

    QByteArray block;
    {
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_11);
        out << quint32(0);
        out << command;
        out << data;
        out.device()->seek(0);
        out << quint32(block.size());
    }
    socket->write(block);
    socket->flush();
}

void PreviewerServer::onNewConnection()
{
    QLocalSocket* pendingConnection = m_server->nextPendingConnection();

    if (m_socket.isNull()) {
        m_blockSize = 0;
        m_socket = pendingConnection;

        m_checkAliveTimer->start();

        connect(m_socket, &QLocalSocket::disconnected, m_checkAliveTimer, &QTimer::stop);
        connect(m_socket, &QLocalSocket::disconnected, this, &PreviewerServer::disconnected);
        connect(m_socket, &QLocalSocket::disconnected, m_socket, &QLocalSocket::deleteLater);
        connect(m_socket, &QLocalSocket::readyRead, this, &PreviewerServer::onReadReady);
        connect(m_socket, qOverload<QLocalSocket::LocalSocketError>(&QLocalSocket::error),
                this, &PreviewerServer::onError);

        emit connected();
    } else {
        send(pendingConnection, PreviewerCommands::Terminate);
        pendingConnection->abort();
    }
}

void PreviewerServer::onReadReady()
{
    static QByteArray buffer;
    buffer.append(m_socket->readAll());

    QByteArray data;
    PreviewerCommands command;

    {
        QDataStream incoming(buffer);
        incoming.setVersion(QDataStream::Qt_5_11);

        if (m_blockSize == 0) {
            if (buffer.size() < (int)sizeof(quint32))
                return;

            incoming >> m_blockSize;
        }

        if (buffer.size() < (int) m_blockSize)
            return;

        if (incoming.atEnd()) {
            m_blockSize = 0;
            return;
        }

        incoming >> command;
        incoming >> data;
    }

    buffer.remove(0, m_blockSize);
    m_blockSize = 0;

    qDebug() << buffer.size() << command << data;

    if (command == PreviewerCommands::ConnectionAlive)
        m_checkAliveTimer->start();
    else
        emit dataArrived(command, data);
}

void PreviewerServer::onError(QLocalSocket::LocalSocketError socketError)
{
    qWarning() << socketError << "Socket error, in" << __FILE__ << ":" << __LINE__;
}

