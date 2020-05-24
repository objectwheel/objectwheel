#include <payloadrelay.h>
#include <hashfactory.h>
#include <utilityfunctions.h>
#include <QWebSocket>

PayloadRelay::PayloadRelay(int payloadSymbol, int payloadAckSymbol, QObject* parent) : QObject(parent)
  , PAYLOAD_SYMBOL(payloadSymbol)
  , PAYLOAD_ACK_SYMBOL(payloadAckSymbol)
  , m_timeout(DEFAULT_TIMEOUT)
  , m_uploadChunkSize(DEFAULT_CHUNK_SIZE)
{
}

PayloadRelay::~PayloadRelay()
{
    qDeleteAll(m_uploads.cbegin(), m_uploads.cend());
    qDeleteAll(m_downloads.cbegin(), m_downloads.cend());
}

bool PayloadRelay::hasUpload(const QString& uid) const
{
    for (const Payload* payload : qAsConst(m_uploads)) {
        if (payload->uid == uid)
            return true;
    }
    return false;
}

bool PayloadRelay::hasDownload(const QString& uid) const
{
    for (const Payload* payload : qAsConst(m_downloads)) {
        if (payload->uid == uid)
            return true;
    }
    return false;
}

int PayloadRelay::timeout() const
{
    return m_timeout;
}

void PayloadRelay::setTimeout(int timeout)
{
    if (m_timeout != timeout) {
        m_timeout = timeout;
        for (Payload* payload : qAsConst(m_uploads))
            payload->timeoutTimer.setInterval(timeout);
        for (Payload* payload : qAsConst(m_downloads))
            payload->timeoutTimer.setInterval(timeout);
    }
}

int PayloadRelay::uploadChunkSize() const
{
    return m_uploadChunkSize;
}

void PayloadRelay::setUploadChunkSize(int uploadChunkSize)
{
    m_uploadChunkSize = uploadChunkSize;
}

QString PayloadRelay::scheduleUpload(QWebSocket* socket, const QByteArray& data)
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
        connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadRelay::onBinaryMessageReceived);

    QTimer::singleShot(100, this, [=] { if (m_uploads.contains(payload)) uploadNextAvailableChunk(payload); });

    m_uploads.append(payload);

    return payload->uid;
}

void PayloadRelay::registerDownload(QWebSocket* socket, const QString& uid)
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
        connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadRelay::onBinaryMessageReceived);

    m_downloads.append(payload);
}

void PayloadRelay::cancelUpload(const QString& uid)
{
    cleanUpload(uploadPayloadFromUid(uid));
}

void PayloadRelay::cancelDownload(const QString& uid)
{
    cleanDownload(downloadPayloadFromUid(uid));
}

void PayloadRelay::uploadNextAvailableChunk(Payload* payload)
{
    if (payload == 0)
        return;

    if (payload->socket->state() != QAbstractSocket::ConnectedState)
        return;

    if (payload->buffer.atEnd())
        return;

    const int oldPos = payload->buffer.pos();
    payload->timeoutTimer.start();
    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                           PAYLOAD_SYMBOL,
                                           payload->uid,
                                           false,
                                           payload->buffer.data().size(),
                                           payload->buffer.read(m_uploadChunkSize)));
    payload->socket->flush();

    emit bytesUploaded(payload->uid, payload->buffer.pos() - oldPos);

    if (payload->buffer.atEnd()) {
        const QString uid = payload->uid;
        cleanUpload(payload);
        emit uploadFinished(uid);
    }
}

void PayloadRelay::downloadNextAvailableChunk(Payload* payload) const
{
    if (payload == 0)
        return;

    if (payload->socket->state() != QAbstractSocket::ConnectedState)
        return;

    Q_ASSERT(payload->buffer.buffer().capacity() > payload->buffer.pos());

    payload->timeoutTimer.start();
    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                           PAYLOAD_ACK_SYMBOL,
                                           payload->uid,
                                           false));
    payload->socket->flush();
}

void PayloadRelay::onBinaryMessageReceived(const QByteArray& message)
{
    int symbol = 0;
    UtilityFunctions::pullCbor(message, symbol);

    if (symbol == PAYLOAD_SYMBOL) {
        QString uid;
        bool cancel = false;
        int totalBytes;
        QByteArray chunk;
        UtilityFunctions::pullCbor(message, symbol, uid, cancel, totalBytes, chunk);

        if (cancel) {
            cancelDownload(uid);
        } else {
            if (Payload* payload = downloadPayloadFromUid(uid)) {
                // First chunk
                if (!payload->buffer.isOpen()) {
                    payload->buffer.buffer().reserve(totalBytes);
                    payload->buffer.open(QBuffer::WriteOnly);
                }

                // Write data into the buffer
                payload->buffer.write(chunk);
                emit bytesDownloaded(uid, chunk, totalBytes);

                // Last chunk
                if (payload->buffer.pos() >= totalBytes) {
                    payload->buffer.close();
                    const QByteArray data = payload->buffer.data();
                    cleanDownload(payload);
                    emit downloadFinished(uid, data);
                } else {
                    downloadNextAvailableChunk(payload);
                }
            } else {
                qWarning("PayloadRelay: Unknown payload arrived");
            }
        }
    } else if (symbol == PAYLOAD_ACK_SYMBOL) {
        QString uid;
        bool cancel = false;
        UtilityFunctions::pullCbor(message, symbol, uid, cancel);
        if (cancel) {
            cancelUpload(uid);
        } else {
            if (Payload* payload = uploadPayloadFromUid(uid))
                uploadNextAvailableChunk(payload);
            else
                qWarning("PayloadRelay: Upload requested for unknown payload uid: %s", uid.toUtf8().constData());
        }
    }
}

int PayloadRelay::socketCount(QWebSocket* socket) const
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

void PayloadRelay::timeoutUpload(Payload* payload)
{
    const QString payloadUid = payload->uid;
    cancelUpload(payloadUid);
    emit uploadTimedout(payloadUid);
}

void PayloadRelay::timeoutDownload(Payload* payload)
{
    const QString payloadUid = payload->uid;
    cancelDownload(payloadUid);
    emit downloadTimedout(payloadUid);
}

void PayloadRelay::cleanUpload(Payload* payload)
{
    if (payload) {
        for (const QMetaObject::Connection& connection : qAsConst(payload->connections))
            QObject::disconnect(connection);
        if (socketCount(payload->socket) == 1)
            payload->socket->disconnect(this);
        if (payload->socket->state() == QAbstractSocket::ConnectedState) {
            payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                                   PAYLOAD_SYMBOL,
                                                   payload->uid,
                                                   true));
            payload->socket->flush();
        }
        m_uploads.removeOne(payload);
        delete payload;
    }
}

void PayloadRelay::cleanDownload(Payload* payload)
{
    if (payload) {
        for (const QMetaObject::Connection& connection : qAsConst(payload->connections))
            QObject::disconnect(connection);
        if (socketCount(payload->socket) == 1)
            payload->socket->disconnect(this);
        m_downloads.removeOne(payload);
        if (payload->socket->state() == QAbstractSocket::ConnectedState) {
            payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                                   PAYLOAD_ACK_SYMBOL,
                                                   payload->uid,
                                                   true));
            payload->socket->flush();
        }
        delete payload;
    }
}

PayloadRelay::Payload* PayloadRelay::uploadPayloadFromUid(const QString& uid) const
{
    if (!uid.isEmpty()) {
        for (Payload* payload : qAsConst(m_uploads)) {
            if (payload->uid == uid)
                return payload;
        }
    }
    return nullptr;
}

PayloadRelay::Payload* PayloadRelay::downloadPayloadFromUid(const QString& uid) const
{
    if (!uid.isEmpty()) {
        for (Payload* payload : qAsConst(m_downloads)) {
            if (payload->uid == uid)
                return payload;
        }
    }
    return nullptr;
}