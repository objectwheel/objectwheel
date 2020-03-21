#include <payloadrelay.h>
#include <hashfactory.h>
#include <utilityfunctions.h>
#include <QWebSocket>

// TODO: Timeout and clear upload download cache

PayloadRelay::PayloadRelay(int payloadSymbol, int payloadAckSymbol, QObject* parent) : QObject(parent)
  , m_payloadSymbol(payloadSymbol)
  , m_payloadAckSymbol(payloadAckSymbol)
{
}

PayloadRelay::~PayloadRelay()
{
    qDeleteAll(m_uploads.cbegin(), m_uploads.cend());
    qDeleteAll(m_downloads.cbegin(), m_downloads.cend());
}

void PayloadRelay::download(QWebSocket* socket, const QString& uid)
{
    if (!uid.isEmpty()) {
        auto payload = new Payload;
        payload->uid = uid;
        payload->socket = socket;
        payload->timeoutTimer.start(TIMEOUT);
        m_downloads.append(payload);

        connect(&payload->timeoutTimer, &QTimer::timeout, this, [=] { cancelDownload(payload->uid); });
        connect(socket, &QWebSocket::destroyed, this, [=] { cancelDownload(payload->uid); });
        connect(socket, &QWebSocket::disconnected, this, [=] { cancelDownload(payload->uid); });
        connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadRelay::onBinaryMessageReceived);
    }
}

QString PayloadRelay::upload(QWebSocket* socket, const QByteArray& data)
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
    payload->timeoutTimer.start(TIMEOUT);
    m_uploads.append(payload);

    connect(&payload->timeoutTimer, &QTimer::timeout, this, [=] { cancelUpload(payload->uid); });
    connect(socket, &QWebSocket::destroyed, this, [=] { cancelUpload(payload->uid); });
    connect(socket, &QWebSocket::disconnected, this, [=] { cancelUpload(payload->uid); });
    connect(socket, &QWebSocket::binaryMessageReceived, this, &PayloadRelay::onBinaryMessageReceived);
    QMetaObject::invokeMethod(this, "uploadNextAvailableChunk", Qt::QueuedConnection, Q_ARG(Payload*, payload));

    return payload->uid;
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
                                           payload->buffer.read(CHUNK_SIZE)));

    emit bytesUploaded(payload->uid, payload->buffer.pos());

    if (payload->buffer.atEnd()) {
        emit uploadFinished(payload->uid);
        cleanUpload(payload);
    }
}

void PayloadRelay::downloadNextAvailableChunk(Payload* payload)
{
    if (payload == 0)
        return;

    if (payload->socket->state() != QAbstractSocket::ConnectedState)
        return;

    Q_ASSERT(payload->buffer.buffer().capacity() > payload->buffer.pos());

    payload->timeoutTimer.start();
    payload->socket->sendBinaryMessage(UtilityFunctions::pushCbor(
                                           m_payloadAckSymbol,
                                           payload->uid,
                                           true));
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
                emit downloadFinished(uid, payload->buffer.data());
                cleanDownload(payload);
            } else {
                downloadNextAvailableChunk(payload);
            }
        }
    } else if (symbol == m_payloadAckSymbol) {
        QString uid;
        bool accepted = false;
        UtilityFunctions::pullCbor(message, symbol, uid, accepted);
        if (accepted)
            uploadNextAvailableChunk(uploadPayloadFromUid(uid));
        else
            cancelUpload(uid);
    }
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

PayloadRelay::Payload* PayloadRelay::uploadPayloadFromUid(const QString& uid)
{
    if (!uid.isEmpty()) {
        for (Payload* payload : qAsConst(m_uploads)) {
            if (payload->uid == uid)
                return payload;
        }
    }
    return nullptr;
}

PayloadRelay::Payload* PayloadRelay::downloadPayloadFromUid(const QString& uid)
{
    if (!uid.isEmpty()) {
        for (Payload* payload : qAsConst(m_downloads)) {
            if (payload->uid == uid)
                return payload;
        }
    }
    return nullptr;
}
