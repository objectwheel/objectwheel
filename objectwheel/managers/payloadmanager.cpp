#include <payloadmanager.h>
#include <hashfactory.h>
#include <utilityfunctions.h>

#include <QWebSocket>
#include <QTcpServer>

PayloadManager::PayloadManager(InstallationType installationType, QObject* parent) : QObject(parent)
  , m_installationType(installationType)
  , m_server(m_installationType == ServerSetup ? new QTcpServer(this) : nullptr)
  , m_timeout(DEFAULT_TIMEOUT)
  , m_uploadChunkSize(DEFAULT_CHUNK_SIZE)
{
}

PayloadManager::~PayloadManager()
{
    qDeleteAll(m_uploads.cbegin(), m_uploads.cend());
    qDeleteAll(m_downloads.cbegin(), m_downloads.cend());
}

bool PayloadManager::hasUpload(const QString& uid) const
{
    for (const Payload* payload : qAsConst(m_uploads)) {
        if (payload->uid == uid)
            return true;
    }
    return false;
}

bool PayloadManager::hasDownload(const QString& uid) const
{
    for (const Payload* payload : qAsConst(m_downloads)) {
        if (payload->uid == uid)
            return true;
    }
    return false;
}

int PayloadManager::timeout() const
{
    return m_timeout;
}

void PayloadManager::setTimeout(int timeout)
{
    if (m_timeout != timeout) {
        m_timeout = timeout;
        for (Payload* payload : qAsConst(m_uploads))
            payload->timeoutTimer.setInterval(timeout);
        for (Payload* payload : qAsConst(m_downloads))
            payload->timeoutTimer.setInterval(timeout);
    }
}

int PayloadManager::uploadChunkSize() const
{
    return m_uploadChunkSize;
}

void PayloadManager::setUploadChunkSize(int uploadChunkSize)
{
    m_uploadChunkSize = uploadChunkSize;
}

QString PayloadManager::scheduleUpload(QWebSocket* socket, const QByteArray& data)
{
    if (data.isEmpty())
        return QString();

    if (socket == 0)
        return QString();

    if (socket->state() != QAbstractSocket::ConnectedState)
        return QString();

    auto payload = new Payload;
    payload->uid = HashFactory::generate();
    payload->socket = socket;
    payload->buffer.setData(data);
    payload->buffer.open(QIODevice::ReadOnly);
    payload->timeoutTimer.start(m_timeout);
    payload->connections.append(connect(&payload->timeoutTimer, &QTimer::timeout, this, [=] { timeoutUpload(payload); }));
    payload->connections.append(connect(socket, &QWebSocket::destroyed, this, [=] { cancelUpload(payload->uid); }));
    payload->connections.append(connect(socket, &QWebSocket::disconnected, this, [=] { cancelUpload(payload->uid); }));

    if (socketCount(socket) == 0)
        connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadManager::onBinaryMessageReceived);

    QTimer::singleShot(100, this, [=] { if (m_uploads.contains(payload)) uploadNextAvailableChunk(payload); });

    m_uploads.append(payload);

    return payload->uid;
}

void PayloadManager::registerDownload(QWebSocket* socket, const QString& uid)
{
    if (uid.isEmpty())
        return;

    if (socket == 0)
        return;

    if (socket->state() != QAbstractSocket::ConnectedState)
        return;

    auto payload = new Payload;
    payload->uid = uid;
    payload->socket = socket;
    payload->timeoutTimer.start(m_timeout);
    payload->connections.append(connect(&payload->timeoutTimer, &QTimer::timeout, this, [=] { timeoutDownload(payload); }));
    payload->connections.append(connect(socket, &QWebSocket::destroyed, this, [=] { cancelDownload(payload->uid); }));
    payload->connections.append(connect(socket, &QWebSocket::disconnected, this, [=] { cancelDownload(payload->uid); }));

    if (socketCount(socket) == 0)
        connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadManager::onBinaryMessageReceived);

    m_downloads.append(payload);
}

void PayloadManager::cancelUpload(const QString& uid)
{
    cleanUpload(uploadPayloadFromUid(uid));
}

void PayloadManager::cancelDownload(const QString& uid)
{
    cleanDownload(downloadPayloadFromUid(uid));
}

void PayloadManager::uploadNextAvailableChunk(Payload* payload)
{
    if (payload == 0)
        return;

    if (payload->socket->state() != QAbstractSocket::ConnectedState)
        return;

    if (payload->buffer.atEnd())
        return;

    const int oldPos = payload->buffer.pos();
    payload->timeoutTimer.start();
//    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
//                                           PAYLOAD_SYMBOL,
//                                           payload->uid,
//                                           false,
//                                           payload->buffer.data().size(),
//                                           payload->buffer.read(m_uploadChunkSize)));
    payload->socket->flush();

    emit bytesUploaded(payload->uid, payload->buffer.pos() - oldPos);

    if (payload->buffer.atEnd()) {
        const QString uid = payload->uid;
        cleanUpload(payload);
        emit uploadFinished(uid);
    }
}

void PayloadManager::downloadNextAvailableChunk(Payload* payload) const
{
    if (payload == 0)
        return;

    if (payload->socket->state() != QAbstractSocket::ConnectedState)
        return;

    Q_ASSERT(payload->buffer.buffer().capacity() > payload->buffer.pos());

    payload->timeoutTimer.start();
//    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
//                                           PAYLOAD_ACK_SYMBOL,
//                                           payload->uid,
//                                           false));
    payload->socket->flush();
}

void PayloadManager::onBinaryMessageReceived(const QByteArray& message)
{
    int symbol = 0;
    UtilityFunctions::pullCbor(message, symbol);

//    if (symbol == PAYLOAD_SYMBOL) {
//        QString uid;
//        bool cancel = false;
//        int totalBytes;
//        QByteArray chunk;
//        UtilityFunctions::pullCbor(message, symbol, uid, cancel, totalBytes, chunk);

//        if (cancel) {
//            cancelDownload(uid);
//        } else {
//            if (Payload* payload = downloadPayloadFromUid(uid)) {
//                // First chunk
//                if (!payload->buffer.isOpen()) {
//                    payload->buffer.buffer().reserve(totalBytes);
//                    payload->buffer.open(QBuffer::WriteOnly);
//                }

//                // Write data into the buffer
//                payload->buffer.write(chunk);
//                emit bytesDownloaded(uid, chunk, totalBytes);

//                // Last chunk
//                if (payload->buffer.pos() >= totalBytes) {
//                    payload->buffer.close();
//                    const QByteArray data = payload->buffer.data();
//                    cleanDownload(payload);
//                    emit downloadFinished(uid, data);
//                } else {
//                    downloadNextAvailableChunk(payload);
//                }
//            } else {
//                qWarning("PayloadManager: Unknown payload arrived");
//            }
//        }
//    } else if (symbol == PAYLOAD_ACK_SYMBOL) {
//        QString uid;
//        bool cancel = false;
//        UtilityFunctions::pullCbor(message, symbol, uid, cancel);
//        if (cancel) {
//            cancelUpload(uid);
//        } else {
//            if (Payload* payload = uploadPayloadFromUid(uid))
//                uploadNextAvailableChunk(payload);
//            else
//                qWarning("PayloadManager: Upload requested for unknown payload uid: %s", uid.toUtf8().constData());
//        }
//    }
}

int PayloadManager::socketCount(QWebSocket* socket) const
{
    int total = 0;
    for (const Payload* payload : qAsConst(m_uploads)) {
        if (payload->socket == socket)
            ++total;
    }
    for (const Payload* payload : qAsConst(m_downloads)) {
        if (payload->socket == socket)
            ++total;
    }
    return total;
}

void PayloadManager::timeoutUpload(Payload* payload)
{
    const QString payloadUid = payload->uid;
    cancelUpload(payloadUid);
    emit uploadTimedout(payloadUid);
}

void PayloadManager::timeoutDownload(Payload* payload)
{
    const QString payloadUid = payload->uid;
    cancelDownload(payloadUid);
    emit downloadTimedout(payloadUid);
}

void PayloadManager::cleanUpload(Payload* payload)
{
    if (payload) {
        for (const QMetaObject::Connection& connection : qAsConst(payload->connections))
            QObject::disconnect(connection);
        if (socketCount(payload->socket) == 1)
            payload->socket->disconnect(this);
        if (payload->socket->state() == QAbstractSocket::ConnectedState) {
//            payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
//                                                   PAYLOAD_SYMBOL,
//                                                   payload->uid,
//                                                   true));
            payload->socket->flush();
        }
        m_uploads.removeOne(payload);
        delete payload;
    }
}

void PayloadManager::cleanDownload(Payload* payload)
{
    if (payload) {
        for (const QMetaObject::Connection& connection : qAsConst(payload->connections))
            QObject::disconnect(connection);
        if (socketCount(payload->socket) == 1)
            payload->socket->disconnect(this);
        m_downloads.removeOne(payload);
        if (payload->socket->state() == QAbstractSocket::ConnectedState) {
//            payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
//                                                   PAYLOAD_ACK_SYMBOL,
//                                                   payload->uid,
//                                                   true));
            payload->socket->flush();
        }
        delete payload;
    }
}

PayloadManager::Payload* PayloadManager::uploadPayloadFromUid(const QString& uid) const
{
    if (!uid.isEmpty()) {
        for (Payload* payload : qAsConst(m_uploads)) {
            if (payload->uid == uid)
                return payload;
        }
    }
    return nullptr;
}

PayloadManager::Payload* PayloadManager::downloadPayloadFromUid(const QString& uid) const
{
    if (!uid.isEmpty()) {
        for (Payload* payload : qAsConst(m_downloads)) {
            if (payload->uid == uid)
                return payload;
        }
    }
    return nullptr;
}
