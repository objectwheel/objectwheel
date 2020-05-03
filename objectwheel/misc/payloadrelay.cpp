#include <payloadrelay.h>
#include <hashfactory.h>
#include <utilityfunctions.h>
#include <QWebSocket>

PayloadRelay::PayloadRelay(int payloadSymbol, int payloadAckSymbol, QObject* parent) : QObject(parent)
  , m_payloadSymbol(payloadSymbol)
  , m_payloadAckSymbol(payloadAckSymbol)
  , m_timeout(DEFAULT_TIMEOUT)
  , m_uploadChunkSize(DEFAULT_CHUNK_SIZE)
  , m_downloadBuffered(true)
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

bool PayloadRelay::downloadBuffered() const
{
    return m_downloadBuffered;
}

void PayloadRelay::setDownloadBuffered(bool downloadBuffered)
{
    if (m_downloadBuffered != downloadBuffered) {
        if (m_downloads.isEmpty())
            m_downloadBuffered = downloadBuffered;
        else
            qWarning("PayloadRelay: Cannot set downloadBuffered while download in progress");
    }
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
    m_uploads.append(payload);

    connect(&payload->timeoutTimer, &QTimer::timeout, this, [=] { timeoutUpload(payload); });
    connect(socket, &QWebSocket::destroyed, this, [=] { cancelUpload(payload->uid); });
    connect(socket, &QWebSocket::disconnected, this, [=] { cancelUpload(payload->uid); });
    connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadRelay::onBinaryMessageReceived);
    QTimer::singleShot(100, this, [=] { if (m_uploads.contains(payload)) uploadNextAvailableChunk(payload); });

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
    payload->receivedBytes = 0;
    payload->uid = uid;
    payload->socket = socket;
    payload->timeoutTimer.start(m_timeout);
    m_downloads.append(payload);

    connect(&payload->timeoutTimer, &QTimer::timeout, this, [=] { timeoutDownload(payload); });
    connect(socket, &QWebSocket::destroyed, this, [=] { cancelDownload(payload->uid); });
    connect(socket, &QWebSocket::disconnected, this, [=] { cancelDownload(payload->uid); });
    connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadRelay::onBinaryMessageReceived);
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

    payload->timeoutTimer.start();
    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                           m_payloadSymbol,
                                           payload->uid,
                                           payload->buffer.size(),
                                           payload->buffer.read(m_uploadChunkSize)));
    payload->socket->flush();

    emit bytesUploaded(payload->uid, payload->buffer.pos());

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

    Q_ASSERT(!m_downloadBuffered || (payload->buffer.buffer().capacity() > payload->buffer.pos()));

    payload->timeoutTimer.start();
    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                           m_payloadAckSymbol,
                                           payload->uid,
                                           true));
    payload->socket->flush();
}

void PayloadRelay::onBinaryMessageReceived(const QByteArray& message)
{
    int symbol = 0;
    UtilityFunctions::pullCbor(message, symbol);

    if (symbol == m_payloadSymbol) {
        QString uid;
        int totalBytes;
        QByteArray chunk;
        UtilityFunctions::pullCbor(message, symbol, uid, totalBytes, chunk);

        if (auto payload = downloadPayloadFromUid(uid)) {
            if (m_downloadBuffered) {
                // First chunk
                if (!payload->buffer.isOpen()) {
                    payload->buffer.buffer().reserve(totalBytes);
                    payload->buffer.open(QBuffer::WriteOnly);
                }

                // Write data into the buffer
                payload->buffer.write(chunk);
            }
            payload->receivedBytes += chunk.size();
            emit bytesDownloaded(uid, chunk, totalBytes);

            // Last chunk
            if (payload->receivedBytes >= totalBytes) {
                if (m_downloadBuffered)
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
    } else if (symbol == m_payloadAckSymbol) {
        QString uid;
        bool accepted = false;
        UtilityFunctions::pullCbor(message, symbol, uid, accepted);
        if (accepted) {
            if (Payload* payload = uploadPayloadFromUid(uid))
                uploadNextAvailableChunk(payload);
            else
                qWarning("PayloadRelay: Upload requested for unknown payload uid");
        } else {
            cancelUpload(uid);
        }
    }
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
        payload->socket->disconnect(this);
        m_uploads.removeOne(payload);
        delete payload;
    }
}

void PayloadRelay::cleanDownload(Payload* payload)
{
    if (payload) {
        payload->socket->disconnect(this);
        m_downloads.removeOne(payload);
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
