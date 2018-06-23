#include <previewerserver.h>

#include <QTimer>
#include <QDataStream>
#include <QLocalServer>
#include <QLocalSocket>

PreviewerServer::PreviewerServer(QObject* parent)
    : QObject(parent)
    , m_blockSize(0)
    , m_server(new QLocalServer(this))
    , m_checkAliveTimer(new QTimer(this))
{
    m_checkAliveTimer->setInterval(3000);

    connect(m_checkAliveTimer, &QTimer::timeout, this, &PreviewerServer::connectionTimeout);

    connect(m_server, &QLocalServer::newConnection, this, &PreviewerServer::onNewConnection);
}

void PreviewerServer::close()
{
    m_server->close();
    m_checkAliveTimer->stop();
    m_socket.clear();
}

void PreviewerServer::listen(const QString& serverName)
{
    m_server->listen(serverName);
}

void PreviewerServer::send(PreviewerCommands command, const QByteArray& data)
{
    send(m_socket, command, data);
}

void PreviewerServer::send(QLocalSocket* socket, PreviewerCommands command, const QByteArray& data)
{
    if (!socket || !socket->isOpen() || !socket->isWritable())
        return;

    QByteArray outgoing;
    QDataStream out(&outgoing, QIODevice::WriteOnly);

    if (data.isEmpty()) {
        out << int(sizeof(command));
        out << command;
    } else {
        out << int(sizeof(command)) + data.size();
        out << command;
        out << data;
    }

    socket->write(outgoing);
    socket->flush();
}

void PreviewerServer::onNewConnection()
{
    QLocalSocket* pendingConnection = m_server->nextPendingConnection();

    if (m_socket.isNull()) {
        m_socket = pendingConnection;

        m_checkAliveTimer->start();

        connect(m_socket, &QLocalSocket::disconnected, m_socket, &QLocalSocket::deleteLater);
        connect(m_socket, &QLocalSocket::disconnected, m_checkAliveTimer, &QTimer::stop);
        connect(m_socket, &QLocalSocket::disconnected, this, &PreviewerServer::disconnected);
        connect(m_socket, &QLocalSocket::readyRead, this, &PreviewerServer::onReadReady);
    } else {
        send(pendingConnection, PreviewerCommands::Terminate);
        pendingConnection->abort();
    }
}

void PreviewerServer::onReadReady()
{
    QDataStream incoming(m_socket);

    if (m_blockSize == 0) {
        if (m_socket->bytesAvailable() < int(sizeof(int)))
            return;

        incoming >> m_blockSize;
    }

    if (m_socket->bytesAvailable() < m_blockSize)
        return;

    if (incoming.atEnd()) {
        m_blockSize = 0;
        return;
    }

    m_blockSize = 0;

    PreviewerCommands command;
    incoming >> command;

    if (command == PreviewerCommands::ConnectionAlive)
        m_checkAliveTimer->start();
    else
        emit dataArrived(command, incoming);
}

