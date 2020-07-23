#include <payloadmanager.h>
#include <hashfactory.h>
#include <coreconstants.h>

#include <QtEndian>
#include <QBuffer>

#if defined(Q_OS_WINDOWS) // For linger, setsockopt()
#  include <Winsock.h>
#else
#  include <sys/socket.h>
#endif

PayloadManager* PayloadManager::s_instance = nullptr;
QVector<PayloadManager::Download*> PayloadManager::s_downloads;
QVector<PayloadManager::Upload*> PayloadManager::s_uploads;

PayloadManager::PayloadManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
}

PayloadManager::~PayloadManager()
{
    QVector<QByteArray> downloadUids(s_downloads.size());
    for (int i = 0; i < s_downloads.size(); ++i) {
        Download* download = s_downloads[i];
        downloadUids[i] = download->uid;
        download->socket->disconnect(this);
        download->socket->abort();
        delete download->socket;
        delete download;
    }
    s_downloads.clear();

    QVector<QByteArray> uploadUids(s_uploads.size());
    for (int i = 0; i < s_uploads.size(); ++i) {
        Upload* upload = s_uploads[i];
        uploadUids[i] = upload->uid;
        upload->socket->disconnect(this);
        upload->socket->abort();
        delete upload->socket;
        delete upload;
    }
    s_uploads.clear();

    for (const QByteArray& uid : qAsConst(downloadUids))
        emit downloadAborted(uid);
    for (const QByteArray& uid : qAsConst(uploadUids))
        emit uploadAborted(uid);

    s_instance = nullptr;
}

PayloadManager* PayloadManager::instance()
{
    return s_instance;
}

void PayloadManager::startDownload(const QByteArray& uid)
{
    Q_ASSERT(uid.size() == 12);

    auto download = new Download;
    download->uid = uid;
    download->socket = new QSslSocket(s_instance);
    download->bytesRead = 0;
    download->totalBytes = -1;
    download->timer.setSingleShot(true);
    download->timer.start(DataTransferTimeout);
    download->socket->setReadBufferSize(ReadBufferSize);
    s_downloads.append(download);

    connect(&download->timer, &QTimer::timeout,
            s_instance, [=] { timeoutDownload(download); });
    connect(download->socket, &QSslSocket::disconnected,
            s_instance, [=] { abortDownload(download); });

#if defined(PAYLOADMANAGER_DEBUG)
    connect(download->socket, &QSslSocket::connected,
            s_instance, [=] { handleConnected(download); });
    download->socket->connectToHost(QStringLiteral("localhost"), CoreConstants::PAYLAOD_PORT);
#else
    connect(download->socket, &QSslSocket::encrypted,
            s_instance, [=] { handleConnected(download); });
    download->socket->connectToHostEncrypted(CoreConstants::API_HOST, CoreConstants::PAYLAOD_PORT);
#endif
}

void PayloadManager::startUpload(const QByteArray& uid, const QByteArray& data)
{
    Q_ASSERT(uid.size() == 12);
    Q_ASSERT(!data.isEmpty());

    auto upload = new Upload;
    upload->uid = uid;
    upload->data = data;
    upload->bytesLeft = data.size();
    upload->socket = new QSslSocket(s_instance);
    upload->timer.setSingleShot(true);
    upload->timer.start(DataTransferTimeout);
    upload->socket->setReadBufferSize(ReadBufferSize);
    s_uploads.append(upload);

    connect(&upload->timer, &QTimer::timeout,
            s_instance, [=] { timeoutUpload(upload); });
    connect(upload->socket, &QSslSocket::disconnected,
            s_instance, [=] { abortUpload(upload); });

#if defined(PAYLOADMANAGER_DEBUG)
    connect(upload->socket, &QSslSocket::connected,
            s_instance, [=] { handleConnected(upload); });
    upload->socket->connectToHost(QStringLiteral("localhost"), CoreConstants::PAYLAOD_PORT);
#else
    connect(upload->socket, &QSslSocket::encrypted,
            s_instance, [=] { handleConnected(upload); });
    upload->socket->connectToHostEncrypted(CoreConstants::API_HOST, CoreConstants::PAYLAOD_PORT);
#endif
}

void PayloadManager::cancelDownload(const QByteArray& uid, bool abort)
{
    cleanDownload(downloadFromUid(uid), abort);
}

void PayloadManager::cancelUpload(const QByteArray& uid, bool abort)
{
    cleanUpload(uploadFromUid(uid), abort);
}

void PayloadManager::handleConnected(Download* download)
{
    Q_ASSERT(download->socket->state() == QAbstractSocket::ConnectedState);
    Q_ASSERT(download->socket->bytesAvailable() <= 0);

    const qintptr socketDescriptor = download->socket->socketDescriptor();
    if (socketDescriptor > 0) {
        linger l = {0, 0};
        ::setsockopt(socketDescriptor, SOL_SOCKET, SO_LINGER, (char*) &l, sizeof(l));
    }

    connect(download->socket, &QSslSocket::readyRead,
            s_instance, [=] { handleReadyRead(download); });

    // Write the header (uid)
    download->socket->write(download->uid);
}

void PayloadManager::handleReadyRead(Download* download)
{
    Q_ASSERT(download->socket->state() == QAbstractSocket::ConnectedState);

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
        download->timer.start(DataTransferTimeout);
        const qint64 available = download->socket->bytesAvailable();
        const bool isLastFrame = download->bytesRead + available >= download->totalBytes;
        if (isLastFrame) {
            QBuffer buffer;
            buffer.setData(download->socket->readAll());
            buffer.open(QIODevice::ReadOnly);
            const qint64 totalBytes = download->totalBytes;
            const QByteArray uid = download->uid;
            cleanDownload(download, false);
            emit s_instance->readyRead(uid, &buffer, totalBytes, true);
        } else {
            emit s_instance->readyRead(download->uid, download->socket, download->totalBytes, false);
            if (s_downloads.contains(download))
                download->bytesRead += available - download->socket->bytesAvailable();
        }
    }
}

void PayloadManager::handleConnected(Upload* upload)
{
    Q_ASSERT(upload->socket->state() == QAbstractSocket::ConnectedState);

    if (upload->socket->bytesAvailable() > 0 || upload->data.isEmpty()) {
        qWarning("PayloadManager: Upload aborted, absurd data received.");
        abortUpload(upload);
        return;
    }

    const qintptr socketDescriptor = upload->socket->socketDescriptor();
    if (socketDescriptor > 0) {
        linger l = {0, 0};
        ::setsockopt(socketDescriptor, SOL_SOCKET, SO_LINGER, (char*) &l, sizeof(l));
    }

    connect(upload->socket, &QSslSocket::readyRead,
            s_instance, [=] { abortUpload(upload); });
#if defined(PAYLOADMANAGER_DEBUG)
    connect(upload->socket, &QSslSocket::bytesWritten,
            s_instance, [=] (qint64 bytes) { handleBytesWritten(upload, bytes); });
#else
    connect(upload->socket, &QSslSocket::encryptedBytesWritten,
            s_instance, [=] (qint64 bytes) { handleBytesWritten(upload, bytes); });
#endif

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
            qWarning() << QLatin1String("PayloadManager: Upload aborted, failed to write data, reason:")
                       << upload->socket->errorString();
            abortUpload(upload);
            return;
        }
        currentPosition += size;
        bytesLeft -= size;
    }
    upload->data.clear();
}

void PayloadManager::handleBytesWritten(Upload* upload, qint64 bytes)
{
    Q_ASSERT(upload->socket->state() == QAbstractSocket::ConnectedState);

    upload->timer.start(DataTransferTimeout);

    // We don't know the total size of the encrypted data written
    // and Qt doesn't help us figuring that information out. Plus
    // there is no way of knowing the size of the plain data that
    // is written in each encryptedBytesWritten() signal cycle. So
    // we count written data size until it reaches out the plain
    // data size that was originally written and stop emitting the
    // bytesWritten() signal before the last frame. Even if we did
    // not encrypt the data, the written data size is going to be
    // bigger anyway (due to header information we are writing).

#if defined(PAYLOADMANAGER_DEBUG)
    const bool isLastFrame = upload->socket->bytesToWrite() <= 0;
#else
    const bool isLastFrame = upload->socket->encryptedBytesToWrite() <= 0;
#endif
    const bool isPseudoLastFrame = upload->bytesLeft - bytes <= 0;
    if (isLastFrame) {
        const qint64 bytesLeft = upload->bytesLeft;
        const QByteArray uid = upload->uid;
        cleanUpload(upload, false);
        emit s_instance->bytesWritten(uid, bytesLeft, true);
    } else if (!isPseudoLastFrame) {
        upload->bytesLeft -= bytes;
        emit s_instance->bytesWritten(upload->uid, bytes, false);
    }
}

void PayloadManager::abortDownload(Download* download)
{
    if (download && s_downloads.contains(download)) {
        const QByteArray uid = download->uid;
        cleanDownload(download);
        emit s_instance->downloadAborted(uid);
    } else {
        qWarning("Cannot abort unknown download");
    }
}

void PayloadManager::abortUpload(Upload* upload)
{
    if (upload && s_uploads.contains(upload)) {
        const QByteArray uid = upload->uid;
        cleanUpload(upload);
        emit s_instance->uploadAborted(uid);
    } else {
        qWarning("Cannot abort unknown upload");
    }
}

void PayloadManager::timeoutDownload(Download* download)
{
    if (download && s_downloads.contains(download)) {
        const QByteArray uid = download->uid;
        cleanDownload(download);
        emit s_instance->downloadTimedout(uid);
    } else {
        qWarning("Cannot timeout unknown download");
    }
}

void PayloadManager::timeoutUpload(Upload* upload)
{
    if (upload && s_uploads.contains(upload)) {
        const QByteArray uid = upload->uid;
        cleanUpload(upload);
        emit s_instance->uploadTimedout(uid);
    } else {
        qWarning("Cannot timeout unknown upload");
    }
}

void PayloadManager::cleanDownload(Download* download, bool abort)
{
    if (download && s_downloads.contains(download)) {
        s_downloads.removeOne(download);
        download->socket->disconnect(s_instance);
        if (abort)
            download->socket->abort();
        else
            download->socket->close();
        download->socket->deleteLater();
        delete download;
    } else {
        qWarning("Cannot clean unknown download");
    }
}

void PayloadManager::cleanUpload(Upload* upload, bool abort)
{
    if (upload && s_uploads.contains(upload)) {
        s_uploads.removeOne(upload);
        upload->socket->disconnect(s_instance);
        if (abort)
            upload->socket->abort();
        else
            upload->socket->close();
        upload->socket->deleteLater();
        delete upload;
    } else {
        qWarning("Cannot clean unknown upload");
    }
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
