#include <payloadmanager.h>
#include <utilityfunctions.h>
#include <hashfactory.h>

#include <QHostAddress>
#include <QtEndian>

static const char g_authKey[] = "_q_PayloadManager_authKey";

PayloadManager* PayloadManager::s_instance = nullptr;
QVector<PayloadManager::Download*> PayloadManager::s_downloads;
QVector<PayloadManager::Upload*> PayloadManager::s_uploads;

PayloadManager::PayloadManager(QObject* parent) : QObject(parent)
{
    Q_ASSERT(s_instance == 0);
    s_instance = this;
}

PayloadManager::~PayloadManager()
{
    for (const Download* download : qAsConst(s_downloads)) {
        if (download->socket) {
            download->socket->disconnect(s_instance);
            download->socket->abort();
        }
        delete download;
    }
    for (const Upload* upload : qAsConst(s_uploads)) {
        if (upload->socket) {
            upload->socket->disconnect(s_instance);
            upload->socket->abort();
        }
        delete upload;
    }
    s_downloads.clear();
    s_uploads.clear();
    s_instance = nullptr;
}

PayloadManager* PayloadManager::instance()
{
    return s_instance;
}

void PayloadManager::registerDownload(const QByteArray& uid)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(uid.size() == 12);
    Q_ASSERT(!hasDownload(uid) && !hasUpload(uid));
    auto download = new Download;
    download->uid = uid;
    download->socket = new QSslSocket(s_instance);
    download->totalBytes = 0;
    download->timer.setSingleShot(true);
    download->timer.start(DataTransferTimeout);
    connect(&download->timer, &QTimer::timeout, s_instance, [=] { timeoutDownload(download); });
    s_downloads.append(download);
#if defined(QT_DEBUG)
    QMetaObject::invokeMethod(download->socket, "connectToHost",
                              Qt::QueuedConnection, Q_ARG(QString, "objectwheel.com"),
                              Q_ARG(quint16, 5455));
#else
    QMetaObject::invokeMethod(download->socket, "connectToHostEncrypted",
                              Qt::QueuedConnection, Q_ARG(QString, "objectwheel.com"),
                              Q_ARG(quint16, 5455));
#endif
}

QByteArray PayloadManager::registerUpload(const QByteArray& data)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(!data.isEmpty());
    auto upload = new Upload;
    upload->uid = HashFactory::generate();
    upload->data = data;
    upload->socket = new QSslSocket(s_instance);
    upload->timer.setSingleShot(true);
    upload->timer.start(DataTransferTimeout);
    connect(&upload->timer, &QTimer::timeout, s_instance, [=] { timeoutUpload(upload); });
    s_uploads.append(upload);

#if defined(QT_DEBUG)
    QMetaObject::invokeMethod(upload->socket, "connectToHost",
                              Qt::QueuedConnection, Q_ARG(QString, "objectwheel.com"),
                              Q_ARG(quint16, 5455));
#else
    QMetaObject::invokeMethod(upload->socket, "connectToHostEncrypted",
                              Qt::QueuedConnection, Q_ARG(QString, "objectwheel.com"),
                              Q_ARG(quint16, 5455));
#endif

    return upload->uid;
}

void PayloadManager::processData(QSslSocket* socket)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(socket);
    Q_ASSERT(socket->state() == QAbstractSocket::ConnectedState);

    Download* download = downloadFromSocket(socket);
    Upload* upload = uploadFromSocket(socket);

    /* Client must send 12 bytes-long binary representation of the UID
     * as the first set of bytes for the initial communication. If not
     * sent in this format, or the UID doesn't match any registered UID
     * then the connection is going to be aborted. Connection will also
     * be aborted if UID authentication is not done within the timeout.
     */

    // Authenticate
    if (download == 0 && upload == 0) {
        if (socket->bytesAvailable() < 12)
            return;

        const QByteArray& uid = socket->peek(12);

        if (!hasDownload(uid) && !hasUpload(uid)) {
            WARNING(QLatin1String("Socket aborted, unregistered uid, address: ") +
                    socket->peerAddress().toString() + QLatin1String(", uid: ") + uid);
            socket->abort();
            return;
        }

        if (hasDownload(uid)) {
            // 12 Bytes for Uid + 8 Bytes for Size = 20 Bytes
            if (socket->bytesAvailable() < 20)
                return;

            socket->skip(12); // Skip Uid
            const QByteArray& size = socket->read(8);
            download = downloadFromUid(uid);
            download->socket = socket;
            download->totalBytes = qFromBigEndian<qint64>(size.constData());
            connect(socket, &QSslSocket::destroyed, s_instance, [=] { cancelDownload(uid); });
        } else {
            socket->skip(12); // Skip Uid
            upload = uploadFromUid(uid);
            upload->socket = socket;
            connect(socket, &QSslSocket::destroyed, s_instance, [=] { cancelUpload(uid); });
#if defined(QT_DEBUG)
            connect(socket, &QSslSocket::bytesWritten,
                    s_instance, [=] (qint64 bytes) { handleBytesWritten(uid, bytes); });
#else
            connect(socket, &QSslSocket::encryptedBytesWritten,
                    s_instance, [=] (qint64 bytes) { handleBytesWritten(uid, bytes); });
#endif
        }

        UtilityFunctions::stopSocketTimeout(socket, g_authKey);
    }

    // Process
    if (download) {
        if (socket->bytesAvailable() > 0) {
            download->timer.start(download->timer.interval());
            emit s_instance->readyRead(download->uid, socket, download->totalBytes);
        }
    } else {
        if (socket->bytesAvailable() > 0 || upload->data.isEmpty()) {
            WARNING(QLatin1String("Socket aborted, absurd data received, address: ") +
                    socket->peerAddress().toString() + QLatin1String(", uid: ") + upload->uid);
            socket->abort();
            return;
        }
        upload->timer.start(upload->timer.interval());
        // Write the header
        const qint64 totalBytes = qToBigEndian<qint64>(upload->data.size());
        socket->write(upload->uid);
        socket->write(reinterpret_cast<const char*>(&totalBytes), 8);
        // Write the body
        qint64 currentPosition = 0;
        qint64 bytesLeft = upload->data.size();
        qint64 numFrames = bytesLeft / FrameSizeInBytes;
        if (bytesLeft % FrameSizeInBytes > 0)
            numFrames++;
        for (qint64 i = 0; i < numFrames; ++i) {
            const qint64 size = qMin(bytesLeft, qint64(FrameSizeInBytes));
            if (socket->write(upload->data.constData() + currentPosition, size) != size) {
                CRITICAL(QLatin1String("Socket aborted, failed to write data, address: ") +
                         socket->peerAddress().toString() + QLatin1String(", uid: ") + upload->uid +
                         QLatin1String(", error: ") + socket->errorString());
                socket->abort();
                return;
            }
            currentPosition += size;
            bytesLeft -= size;
        }
        upload->data.clear();
    }
}

void PayloadManager::handleBytesWritten(const QByteArray& uid, qint64 bytes)
{
    Q_ASSERT(s_instance);
    if (Upload* upload = uploadFromUid(uid)) {
        upload->timer.start(upload->timer.interval());
        emit s_instance->bytesWritten(uid, bytes);
    }
}

void PayloadManager::cancelDownload(const QByteArray& uid)
{
    Q_ASSERT(s_instance);
    if (Download* download = downloadFromUid(uid)) {
        s_downloads.removeOne(download);
        if (download->socket) {
            download->socket->disconnect(s_instance);
            download->socket->abort();
        }
        delete download;
    }
}

void PayloadManager::cancelUpload(const QByteArray& uid)
{
    Q_ASSERT(s_instance);
    if (Upload* upload = uploadFromUid(uid)) {
        s_uploads.removeOne(upload);
        if (upload->socket) {
            upload->socket->disconnect(s_instance);
            upload->socket->abort();
        }
        delete upload;
    }
}

void PayloadManager::timeoutDownload(const Download* download)
{
    Q_ASSERT(s_instance);
    if (download && hasDownload(download->uid)) {
        const QByteArray uid = download->uid;
        cancelDownload(uid);
        emit s_instance->downloadTimedout(uid);
    }
}

void PayloadManager::timeoutUpload(const Upload* upload)
{
    Q_ASSERT(s_instance);
    if (upload && hasUpload(upload->uid)) {
        const QByteArray uid = upload->uid;
        cancelUpload(uid);
        emit s_instance->uploadTimedout(uid);
    }
}

int PayloadManager::simultaneousConnectionCount(const QSslSocket* socket)
{
    if (socket == 0)
        return 0;
    int total = 0;
    for (const Download* download : qAsConst(s_downloads)) {
        if (download->socket && download->socket->peerAddress() == socket->peerAddress())
            total++;
    }
    for (const Upload* upload : qAsConst(s_uploads)) {
        if (upload->socket && upload->socket->peerAddress() == socket->peerAddress())
            total++;
    }
    return total;
}

bool PayloadManager::hasDownload(const QByteArray& uid)
{
    if (uid.size() != 12)
        return false;
    for (const Download* download : qAsConst(s_downloads)) {
        if (download->uid == uid)
            return true;
    }
    return false;
}

bool PayloadManager::hasDownload(const QSslSocket* socket)
{
    if (socket == 0)
        return false;
    for (const Download* download : qAsConst(s_downloads)) {
        if (download->socket == socket)
            return true;
    }
    return false;
}

bool PayloadManager::hasUpload(const QByteArray& uid)
{
    if (uid.size() != 12)
        return false;
    for (const Upload* upload : qAsConst(s_uploads)) {
        if (upload->uid == uid)
            return true;
    }
    return false;
}

bool PayloadManager::hasUpload(const QSslSocket* socket)
{
    if (socket == 0)
        return false;
    for (const Upload* upload : qAsConst(s_uploads)) {
        if (upload->socket == socket)
            return true;
    }
    return false;
}

PayloadManager::Download* PayloadManager::downloadFromUid(const QByteArray& uid)
{
    if (uid.size() != 12)
        return nullptr;
    for (Download* download : qAsConst(s_downloads)) {
        if (download->uid == uid)
            return download;
    }
    return nullptr;
}

PayloadManager::Download* PayloadManager::downloadFromSocket(const QSslSocket* socket)
{
    if (socket == 0)
        return nullptr;
    for (Download* download : qAsConst(s_downloads)) {
        if (download->socket == socket)
            return download;
    }
    return nullptr;
}

PayloadManager::Upload* PayloadManager::uploadFromUid(const QByteArray& uid)
{
    if (uid.size() != 12)
        return nullptr;
    for (Upload* upload : qAsConst(s_uploads)) {
        if (upload->uid == uid)
            return upload;
    }
    return nullptr;
}

PayloadManager::Upload* PayloadManager::uploadFromSocket(const QSslSocket* socket)
{
    if (socket == 0)
        return nullptr;
    for (Upload* upload : qAsConst(s_uploads)) {
        if (upload->socket == socket)
            return upload;
    }
    return nullptr;
}
