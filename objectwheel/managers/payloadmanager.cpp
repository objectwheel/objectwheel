#include <payloadmanager.h>
#include <hashfactory.h>
#include <QtEndian>

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
        download->socket->disconnect(s_instance);
        download->socket->abort();
        delete download->socket;
        delete download;
    }
    for (const Upload* upload : qAsConst(s_uploads)) {
        upload->socket->disconnect(s_instance);
        upload->socket->abort();
        delete upload->socket;
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
    auto download = new Download;
    download->uid = uid;
    download->socket = new QSslSocket(s_instance);
    download->totalBytes = -1;
    download->timer.setSingleShot(true);
    download->timer.start(DataTransferTimeout);
    connect(&download->timer, &QTimer::timeout,
            s_instance, [=] { timeoutDownload(download); });
    connect(download->socket, &QSslSocket::disconnected,
            s_instance, [=] { cancelDownload(download->uid); });
    connect(download->socket, &QSslSocket::readyRead,
            s_instance, [=] { handleReadyRead(download); }, Qt::QueuedConnection);
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
    connect(&upload->timer, &QTimer::timeout,
            s_instance, [=] { timeoutUpload(upload); });
    connect(upload->socket, &QSslSocket::disconnected,
            s_instance, [=] { cancelUpload(upload->uid); });
    connect(upload->socket, &QSslSocket::readyRead,
            s_instance, [=] { cancelUpload(upload->uid); }, Qt::QueuedConnection);
    s_uploads.append(upload);
#if defined(QT_DEBUG)
    connect(upload->socket, &QSslSocket::connected,
            s_instance, [=] { handleConnected(upload); });
    connect(upload->socket, &QSslSocket::bytesWritten,
            s_instance, [=] (qint64 bytes) { handleBytesWritten(upload, bytes); });
    QMetaObject::invokeMethod(upload->socket, "connectToHost",
                              Qt::QueuedConnection, Q_ARG(QString, QStringLiteral("objectwheel.com")),
                              Q_ARG(quint16, 5455));
#else
    connect(upload->socket, &QSslSocket::encrypted,
            s_instance, [=] { handleConnected(upload); });
    connect(upload->socket, &QSslSocket::encryptedBytesWritten,
            s_instance, [=] (qint64 bytes) { handleBytesWritten(upload, bytes); });
    QMetaObject::invokeMethod(upload->socket, "connectToHostEncrypted",
                              Qt::QueuedConnection, Q_ARG(QString, QStringLiteral("objectwheel.com")),
                              Q_ARG(quint16, 5455));
#endif
    return upload->uid;
}

void PayloadManager::handleReadyRead(Download* download)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(download);

    if (download->totalBytes < 0) {
        // 8 Bytes for Size
        if (download->socket->bytesAvailable() < 8)
            return;

        const QByteArray& size = download->socket->read(8);
        download->totalBytes = qFromBigEndian<qint64>(size.constData());
        Q_ASSERT(download->totalBytes > 0);
    }

    download->timer.start(download->timer.interval());
    emit s_instance->readyRead(download->uid, download->socket, download->totalBytes);
}

void PayloadManager::handleConnected(Upload* upload)
{
    Q_ASSERT(upload);
    Q_ASSERT(!upload->data.isEmpty());

    upload->timer.start(upload->timer.interval());
    // Write the header
    const qint64 totalBytes = qToBigEndian<qint64>(upload->data.size());
    upload->socket->write(upload->uid);
    upload->socket->write(reinterpret_cast<const char*>(&totalBytes), 8);
    // Write the body
    qint64 currentPosition = 0;
    qint64 bytesLeft = upload->data.size();
    qint64 numFrames = bytesLeft / FrameSizeInBytes;
    if (bytesLeft % FrameSizeInBytes > 0)
        numFrames++;
    for (qint64 i = 0; i < numFrames; ++i) {
        const qint64 size = qMin(bytesLeft, qint64(FrameSizeInBytes));
        if (upload->socket->write(upload->data.constData() + currentPosition, size) != size) {
            qWarning() << QLatin1String("Socket aborted, failed to write data, reason:")
                       << upload->socket->errorString();
            upload->socket->abort();
            return;
        }
        currentPosition += size;
        bytesLeft -= size;
    }
    upload->data.clear();
}

void PayloadManager::handleBytesWritten(Upload* upload, qint64 bytes)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(upload);
    upload->timer.start(upload->timer.interval());
    emit s_instance->bytesWritten(upload->uid, bytes);
}

void PayloadManager::cancelDownload(const QByteArray& uid)
{
    Q_ASSERT(s_instance);
    if (Download* download = downloadFromUid(uid)) {
        s_downloads.removeOne(download);
        download->socket->disconnect(s_instance);
        download->socket->abort();
        delete download->socket;
        delete download;
    }
}

void PayloadManager::cancelUpload(const QByteArray& uid)
{
    Q_ASSERT(s_instance);
    if (Upload* upload = uploadFromUid(uid)) {
        s_uploads.removeOne(upload);
        upload->socket->disconnect(s_instance);
        upload->socket->abort();
        delete upload->socket;
        delete upload;
    }
}

void PayloadManager::timeoutDownload(const Download* download)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(download);
    const QByteArray uid = download->uid;
    cancelDownload(uid);
    emit s_instance->downloadTimedout(uid);
}

void PayloadManager::timeoutUpload(const Upload* upload)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(upload);
    const QByteArray uid = upload->uid;
    cancelUpload(uid);
    emit s_instance->uploadTimedout(uid);
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
