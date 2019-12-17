#include <rendersocket.h>
#include <QTimer>
#include <QDataStream>

RenderSocket::RenderSocket(QObject* parent) : QObject(parent)
  , m_blockSize(0)
  , m_socket(new QLocalSocket(this))
  , m_reconnectionTimer(new QTimer(this))
  , m_sendAliveTimer(new QTimer(this))
{
    m_reconnectionTimer->setInterval(1000);
    m_sendAliveTimer->setInterval(800);

    connect(m_socket, &QLocalSocket::readyRead, this, &RenderSocket::onReadReady);
    connect(m_socket, &QLocalSocket::disconnected, this, &RenderSocket::disconnected);
    connect(m_socket, qOverload<QLocalSocket::LocalSocketError>(&QLocalSocket::error),
            this, &RenderSocket::onError);

    connect(m_socket, &QLocalSocket::connected, [=] { m_blockSize = 0; });
    connect(m_socket, &QLocalSocket::connected, m_sendAliveTimer, qOverload<>(&QTimer::start));
    connect(m_socket, &QLocalSocket::disconnected, m_sendAliveTimer, &QTimer::stop);
    connect(m_sendAliveTimer, &QTimer::timeout, this, &RenderSocket::sendAlive);

    connect(m_socket, &QLocalSocket::connected, m_reconnectionTimer, &QTimer::stop);
    connect(m_socket, &QLocalSocket::disconnected, m_reconnectionTimer, qOverload<>(&QTimer::start));
    connect(m_reconnectionTimer, &QTimer::timeout,
            this, static_cast<void(RenderSocket::*)()>(&RenderSocket::connect));
}

void RenderSocket::start(const QString& serverName)
{
    m_serverName = serverName;
    m_reconnectionTimer->start();

    connect();
}

QLocalSocket::LocalSocketState RenderSocket::state()
{
    return m_socket->state();
}

void RenderSocket::abort()
{
    m_reconnectionTimer->stop();
    m_socket->abort();
}

void RenderSocket::stop()
{
    m_reconnectionTimer->stop();
    m_socket->close();
}

void RenderSocket::send(const RendererCommands& command, const QByteArray& data)
{
    if (!m_socket->isOpen() || !m_socket->isWritable())
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
    m_socket->write(block);
    m_socket->flush();
}

void RenderSocket::connect()
{
    m_socket->connectToServer(m_serverName, QIODevice::ReadWrite | QIODevice::Unbuffered);
}

void RenderSocket::sendAlive()
{
    if (state() == QLocalSocket::ConnectedState)
        send(RendererCommands::ConnectionAlive);
}

void RenderSocket::onReadReady()
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

    emit dataArrived(command, data);

    if (m_socket->bytesAvailable() >= (int)sizeof(quint32))
        onReadReady();
}

void RenderSocket::onError(QLocalSocket::LocalSocketError socketError)
{
    qWarning() << socketError << "Socket error, in" << __FILE__ << ":" << __LINE__;
}