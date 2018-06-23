#include <previewerserver.h>
#include <previewercommands.h>

#include <QTimer>
#include <QDataStream>

PreviewerServer::PreviewerServer(QObject* parent)
    : QObject(parent)
    , m_blockSize(0)
    , m_server(new QLocalServer(this))
    , m_checkAliveTimer(new QTimer(this))
{
    m_checkAliveTimer->setInterval(3000);

    connect(m_checkAliveTimer, &QTimer::timeout, this, &PreviewerServer::connectionTimeout);

    connect(m_server, &QLocalServer::newConnection, &PreviewerServer::onNewConnection);
}

void PreviewerServer::listen(const QString& serverName)
{
    m_server->listen(serverName);
}

void PreviewerServer::send(int code, const QByteArray& data)
{
    if (!m_socket->isOpen() || !m_socket->isWritable())
        return;

    QByteArray outgoing;
    QDataStream out(&outgoing, QIODevice::WriteOnly);

    if (data.isEmpty()) {
        out << int(sizeof(int));
        out << code;
    } else {
        out << int(sizeof(int)) + data.size();
        out << code;
        out << data;
    }

    m_socket->write(outgoing);
    m_socket->flush();
}

void PreviewerServer::sendAlive()
{
    if (state() == QLocalSocket::ConnectedState)
        send(CONNECTION_ALIVE);
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

    int code;
    incoming >> code;

    emit dataArrived(code, incoming);

    m_blockSize = 0;
}


void PreviewerServer::onNewConnection()
{
    auto client = m_server->nextPendingConnection();

    if (m_socket.isNull()) {
        m_socket = client;

        connect(m_socket.data(), &QLocalSocket::disconnected, m_socket.data(), &QLocalSocket::deleteLater);
        connect(m_socket.data(), &QLocalSocket::readyRead, &ControlPreviewingManager::onReadReady);

        processNextTask();
    } else {
        terminate(client);
    }
}
