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
        download->socket->disconnect(this);
        if (download->socket->state() != QAbstractSocket::UnconnectedState)
            download->socket->abort();
        download->socket->deleteLater();
        delete download;
    }
    for (const Upload* upload : qAsConst(s_uploads)) {
        upload->socket->disconnect(this);
        if (upload->socket->state() != QAbstractSocket::UnconnectedState)
            upload->socket->abort();
        upload->socket->deleteLater();
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

void PayloadManager::scheduleDownload(const QByteArray& uid)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(uid.size() == 12);

    auto download = new Download;
    download->uid = uid;
    download->socket = new QSslSocket(s_instance);
    download->totalBytes = -1;
    download->bytesWritten = 0;
    download->timer.setSingleShot(true);
    download->timer.start(DataTransferTimeout);
    s_downloads.append(download);

    connect(&download->timer, &QTimer::timeout,
            s_instance, [=] { timeoutDownload(download); });
    connect(download->socket, &QSslSocket::disconnected,
            s_instance, [=] { cancelDownload(download->uid); });
    connect(download->socket, &QSslSocket::readyRead,
            s_instance, [=] { handleReadyRead(download); });

#if defined(PAYLOADMANAGER_DEBUG)
    connect(download->socket, &QSslSocket::connected,
            s_instance, [=] { handleEncrypted(download); });
    download->socket->connectToHost(QStringLiteral("localhost"), 5455);
#else
    connect(download->socket, &QSslSocket::encrypted,
            s_instance, [=] { handleEncrypted(download); });
    download->socket->connectToHostEncrypted(QStringLiteral("objectwheel.com"), 5455);
#endif
}

QByteArray PayloadManager::scheduleUpload(const QByteArray& data)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(!data.isEmpty());

    auto upload = new Upload;
    upload->uid = HashFactory::generate();
    upload->data = data;
    upload->bytesLeft = data.size();
    upload->isSizeCorrected = false;
    upload->socket = new QSslSocket(s_instance);
    upload->timer.setSingleShot(true);
    upload->timer.start(DataTransferTimeout);
    s_uploads.append(upload);

    connect(&upload->timer, &QTimer::timeout,
            s_instance, [=] { timeoutUpload(upload); });
    connect(upload->socket, &QSslSocket::disconnected,
            s_instance, [=] { cancelUpload(upload->uid); });
    connect(upload->socket, &QSslSocket::readyRead,
            s_instance, [=] { cancelUpload(upload->uid); });

#if defined(PAYLOADMANAGER_DEBUG)
    connect(upload->socket, &QSslSocket::connected,
            s_instance, [=] { handleEncrypted(upload); });
    connect(upload->socket, &QSslSocket::bytesWritten,
            s_instance, [=] (qint64 bytes) { handleBytesWritten(upload, bytes); });
    QTimer::singleShot(250, upload->socket, [=] {
        upload->socket->connectToHost(QStringLiteral("localhost"), 5455);
    });
#else
    connect(upload->socket, &QSslSocket::encrypted,
            s_instance, [=] { handleEncrypted(upload); });
    connect(upload->socket, &QSslSocket::encryptedBytesWritten,
            s_instance, [=] (qint64 bytes) { handleBytesWritten(upload, bytes); });
    QTimer::singleShot(250, upload->socket, [=] {
        upload->socket->connectToHostEncrypted(QStringLiteral("objectwheel.com"), 5455);
    });
#endif

    return upload->uid;
}

void PayloadManager::handleReadyRead(Download* download)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(download);
    Q_ASSERT(s_downloads.contains(download));

    // Header
    if (download->totalBytes < 0) {
        // 8 Bytes for Size
        if (download->socket->bytesAvailable() < 8)
            return;

        const QByteArray& size = download->socket->read(8);
        download->totalBytes = qFromBigEndian<qint64>(size.constData());
        Q_ASSERT(download->totalBytes > 0);
    }

    // Body
    if (download->socket->bytesAvailable() > 0) {
        download->timer.start(download->timer.interval());        
        const qint64 available = download->socket->bytesAvailable();
        emit s_instance->readyRead(download->uid, download->socket, download->totalBytes,
                                   download->bytesWritten + available >= download->totalBytes);
        if (s_downloads.contains(download))
            download->bytesWritten += available - download->socket->bytesAvailable();
    }
}

void PayloadManager::handleEncrypted(Upload* upload)
{
    Q_ASSERT(upload);
    Q_ASSERT(!upload->data.isEmpty());
    Q_ASSERT(s_uploads.contains(upload));

    // Write the header (uid + size)
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
    Q_ASSERT(s_uploads.contains(upload));
    if (!upload->isSizeCorrected) {
        upload->isSizeCorrected = true;
        bytes -= 20; // Correct sent bytes size (12 uid + 8 size = 20 bytes)
    }
    upload->timer.start(upload->timer.interval());
    upload->bytesLeft -= bytes;
    emit s_instance->bytesWritten(upload->uid, bytes, upload->bytesLeft <= 0);
}

void PayloadManager::cancelDownload(const QByteArray& uid, bool abort)
{
    Q_ASSERT(s_instance);
    if (Download* download = downloadFromUid(uid)) {
        s_downloads.removeOne(download);
        download->socket->disconnect(s_instance);
        if (download->socket->state() != QAbstractSocket::UnconnectedState) {
            if (abort)
                download->socket->abort();
            else
                download->socket->close();
        }
        download->socket->deleteLater();
        delete download;
    }
}

void PayloadManager::cancelUpload(const QByteArray& uid, bool abort)
{
    Q_ASSERT(s_instance);
    if (Upload* upload = uploadFromUid(uid)) {
        s_uploads.removeOne(upload);
        upload->socket->disconnect(s_instance);
        if (upload->socket->state() != QAbstractSocket::UnconnectedState) {
            if (abort)
                upload->socket->abort();
            else
                upload->socket->close();
        }
        upload->socket->deleteLater();
        delete upload;
    }
}

void PayloadManager::handleEncrypted(Download* download)
{
    Q_ASSERT(download);
    Q_ASSERT(download->uid.size() == 12);
    Q_ASSERT(s_downloads.contains(download));
    // Write the header (uid)
    download->socket->write(download->uid);
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
