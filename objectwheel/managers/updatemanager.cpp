#include <updatemanager.h>
#include <coreconstants.h>
#include <async.h>
#include <systemsettings.h>
#include <updatesettings.h>
#include <applicationcore.h>
#include <servermanager.h>
#include <fastdownloader.h>

#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QFuture>
#include <QSaveFile>

using QCH = QCryptographicHash;

UpdateManager* UpdateManager::s_instance = nullptr;
bool UpdateManager::s_isUpdateCheckRunning = false;
QBuffer UpdateManager::s_checksumsBuffer;
QBuffer UpdateManager::s_changelogBuffer;
FastDownloader UpdateManager::s_checksumsDownloader;
FastDownloader UpdateManager::s_changelogDownloader;
QCborMap UpdateManager::s_localChecksums;
QCborMap UpdateManager::s_remoteChecksums;
QCborMap UpdateManager::s_checksumsDiff;
QString UpdateManager::s_changelog;
qint64 UpdateManager::s_downloadSize = 0;
QFutureWatcher<int> UpdateManager::s_downloadWatcher;
QFutureWatcher<QCborMap> UpdateManager::s_localChecksumsWatcher;

UpdateManager::UpdateManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(ServerManager::instance(), &ServerManager::connected,
            this, &UpdateManager::onConnect, Qt::QueuedConnection);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &UpdateManager::onDisconnect, Qt::QueuedConnection);
    connect(&s_localChecksumsWatcher, &QFutureWatcher<QCborMap>::resultsReadyAt,
            this, &UpdateManager::onLocalScanFinish);
    connect(this, &UpdateManager::updateCheckFinished,
            this, &UpdateManager::onUpdateCheckFinish);
    connect(&s_checksumsDownloader, &FastDownloader::resolved,
            this, &UpdateManager::onChecksumsDownloaderResolved);
    connect(&s_checksumsDownloader, &FastDownloader::readyRead,
            this, &UpdateManager::onChecksumsDownloaderReadyRead);
    connect(&s_checksumsDownloader, qOverload<>(&FastDownloader::finished),
            this, &UpdateManager::onChecksumsDownloaderFinished);
    connect(&s_changelogDownloader, &FastDownloader::resolved,
            this, &UpdateManager::onChangelogDownloaderResolved);
    connect(&s_changelogDownloader, &FastDownloader::readyRead,
            this, &UpdateManager::onChangelogDownloaderReadyRead);
    connect(&s_changelogDownloader, qOverload<>(&FastDownloader::finished),
            this, &UpdateManager::onChangelogDownloaderFinished);
    connect(&s_downloadWatcher, &QFutureWatcher<int>::finished,
            this, &UpdateManager::onDownloadWatcherFinish);
    connect(&s_downloadWatcher, &QFutureWatcher<int>::progressValueChanged,
            this, &UpdateManager::onDownloadWatcherProgressValueChange);

    // WARNING: Clean up update artifacts from previous install if there is any
    QFile::remove(QCoreApplication::applicationDirPath() + QLatin1String("/Updater.bak"));

    s_checksumsDownloader.setNumberOfSimultaneousConnections(2);
    s_changelogDownloader.setNumberOfSimultaneousConnections(2);

    s_checksumsDownloader.setUrl(QUrl(topUpdateRemotePath() + QStringLiteral("/checksums.cbor")));
    s_changelogDownloader.setUrl(QUrl(topUpdateRemotePath() + QStringLiteral("/changelog.html")));

    QFile file(ApplicationCore::updatesPath() + QLatin1String("/Checksums.cbor"));
    if (file.open(QFile::ReadOnly))
        s_localChecksums = QCborValue::fromCbor(file.readAll()).toMap();
}

UpdateManager::~UpdateManager()
{
    if (s_checksumsDownloader.isRunning())
        s_checksumsDownloader.abort();
    if (s_changelogDownloader.isRunning())
        s_changelogDownloader.abort();
    if (s_localChecksumsWatcher.isRunning()) {
        s_localChecksumsWatcher.cancel();
        s_localChecksumsWatcher.waitForFinished();
    }
    if (s_downloadWatcher.isRunning()) {
        s_downloadWatcher.cancel();
        s_downloadWatcher.waitForFinished();
    }
    s_checksumsBuffer.buffer().clear();
    s_changelogBuffer.buffer().clear();
    s_changelog.clear();
    s_checksumsDiff.clear();
    s_remoteChecksums.clear();
    s_localChecksums.clear();
    s_downloadSize = 0;
    s_isUpdateCheckRunning = false;
    s_instance = nullptr;
}

UpdateManager* UpdateManager::instance()
{
    return s_instance;
}

QString UpdateManager::changelog()
{
    return s_changelog;
}

bool UpdateManager::isUpdateCheckRunning()
{
    return s_isUpdateCheckRunning;
}

qint64 UpdateManager::downloadSize()
{
    return s_downloadSize;
}

void UpdateManager::startUpdateCheck(bool force)
{
    Q_ASSERT(ServerManager::isConnected() && !s_isUpdateCheckRunning);

    if (s_changelog.isEmpty() || s_remoteChecksums.isEmpty() || force) {
        if (!s_checksumsDownloader.start()) {
            qWarning("Cannot start downloading update checksums");
            return;
        }
        if (!s_changelogDownloader.start()) {
            s_checksumsDownloader.abort();
            qWarning("Cannot start downloading changelog data");
            return;
        }
        if (force)
            s_localChecksums.clear();
        s_isUpdateCheckRunning = true;
        emit instance()->updateCheckStarted();
    }
}

void UpdateManager::download()
{
    Q_ASSERT(s_downloadWatcher.isFinished());
    s_downloadWatcher.setFuture(Async::run(QThreadPool::globalInstance(), &UpdateManager::handleDownload));
}

void UpdateManager::cancelDownload()
{
    Q_ASSERT(s_downloadWatcher.isRunning());
    s_downloadWatcher.cancel();
}

void UpdateManager::install()
{
    Q_ASSERT(s_downloadWatcher.isFinished() && !s_checksumsDiff.isEmpty());

    QSaveFile file(ApplicationCore::updatesPath() + QLatin1String("/ChecksumsDiff.cbor"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("Cannot open checksums diff file to save");
        return;
    }
    file.write(s_checksumsDiff.toCborValue().toCbor());
    file.commit();

    QProcess::startDetached(QCoreApplication::applicationDirPath() + QLatin1String("/Updater"),
                            QStringList(ApplicationCore::updatesPath() + QLatin1String("/ChecksumsDiff.cbor")));
    QCoreApplication::quit();
}

QDir UpdateManager::topUpdateDir()
{
    // TODO: Handle other OSes
#if defined(Q_OS_MACOS)
    return QFileInfo(QCoreApplication::applicationDirPath() + QStringLiteral("/../..")).canonicalFilePath();
#elif defined(Q_OS_WINDOWS)
    return QCoreApplication::applicationDirPath();
#elif defined(Q_OS_LINUX)
    return QCoreApplication::applicationDirPath();
#endif
    return QString();
}

QString UpdateManager::topUpdateRemotePath()
{
    // TODO: Handle other OSes
#if defined(Q_OS_MACOS)
    return CoreConstants::UPDATE_ADDRESS + QStringLiteral("/macos-x64");
#elif defined(Q_OS_WINDOWS)
    return CoreConstants::UPDATE_ADDRESS + QStringLiteral("/windows-x64");
#elif defined(Q_OS_LINUX)
    return CoreConstants::UPDATE_ADDRESS + QStringLiteral("/linux-x64");
#endif
    return QString();
}

QCborMap UpdateManager::generateUpdateChecksums(const QDir& topDir, const QDir& dir)
{
    QCborMap checksums;
    foreach (const QFileInfo& info, dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (info.isDir()) {
            const QCborMap& map = generateUpdateChecksums(topDir, info.absoluteFilePath());
            if (!map.isEmpty() && map.contains(QCborValue::Undefined))
                return map;
            foreach (const QCborValue& key, map.keys())
                checksums.insert(key, map[key]);
        } else {
            QFile file(info.absoluteFilePath());
            if (!file.open(QFile::ReadOnly)) {
                qWarning() << "WARNING: Cannot open the file for reading, path:"<< info.absoluteFilePath();
                return QCborMap({{QCborValue::Undefined, QCborValue::Undefined}});
            }
            QCborMap map;
            map.insert(QStringLiteral("size"), info.size());
            map.insert(QStringLiteral("sha1"), QCH::hash(file.readAll(), QCH::Sha1));
            checksums.insert(topDir.relativeFilePath(info.absoluteFilePath()), map);
        }
    }
    return checksums;
}

int UpdateManager::handleDownload(QFutureInterfaceBase* futureInterface)
{
    auto future = static_cast<QFutureInterface<int>*>(futureInterface);
    future->setProgressRange(0, 100);
    future->setProgressValue(0);
    QFutureWatcher<int> watcher;
    watcher.setFuture(future->future());

    bool errorSet = false;
    qint64 downloadedSize = 0;
    foreach (const QCborValue& key, s_checksumsDiff.keys()) {
        if (s_checksumsDiff.value(key).toBool(true))
            continue;

        const QString& localPath = ApplicationCore::updatesPath() + QStringLiteral("/Downloads/") + key.toString();
        const QString& remotePath = topUpdateRemotePath() + QStringLiteral("/content/") + key.toString();
        const QCborMap& remote = s_remoteChecksums.value(key).toMap();
        const QByteArray& fileHash = remote.value(QStringLiteral("sha1")).toByteArray();
        const qint64 fileSize = remote.value(QStringLiteral("size")).toInteger();

        if (QFileInfo::exists(localPath)) {
            QFile file(localPath);
            if (!file.open(QFile::ReadOnly)) {
                future->setProgressValueAndText(0, tr("Cannot read update file"));
                return 0;
            }
            if (fileHash == QCH::hash(file.readAll(), QCH::Sha1)) {
                downloadedSize += fileSize;
                future->setProgressValue(100 * downloadedSize / qreal(s_downloadSize));
                continue;
            }
        }

        QEventLoop loop;
        QBuffer buffer;
        buffer.open(QBuffer::WriteOnly);
        FastDownloader downloader(QUrl{remotePath});

        connect(&watcher, &QFutureWatcher<int>::canceled, &loop, &QEventLoop::quit);
        connect(&downloader, &FastDownloader::readyRead, [&] (int id) {
            const qint64 pos = downloader.head(id) + downloader.pos(id);
            const QByteArray& chunk = downloader.readAll(id);
            if (!buffer.seek(pos)) {
                errorSet = true;
                future->setProgressValueAndText(0, tr("Cannot seek further in the buffer"));
                return loop.quit();
            }
            if (buffer.write(chunk) != chunk.size()) {
                errorSet = true;
                future->setProgressValueAndText(0, tr("Cannot write more data into the buffer"));
                return loop.quit();
            }
            downloadedSize += chunk.size();
            future->setProgressValue(100 * downloadedSize / qreal(s_downloadSize));
        });
        connect(&downloader, qOverload<>(&FastDownloader::finished), [&] {
            buffer.close();
            if (downloader.bytesReceived() > 0 && !downloader.isError()) {
                if (fileHash != QCH::hash(buffer.data(), QCH::Sha1)) {
                    errorSet = true;
                    future->setProgressValueAndText(0, tr("Hashes do not match"));
                    return loop.quit();
                }
                if (!QFileInfo(localPath).dir().mkpath(".")) {
                    errorSet = true;
                    future->setProgressValueAndText(0, tr("Cannot establish an update path"));
                    return loop.quit();
                }
                QSaveFile file(localPath);
                if (!file.open(QFile::WriteOnly)) {
                    errorSet = true;
                    future->setProgressValueAndText(0, tr("Cannot open an update file"));
                    return loop.quit();
                }
                if (file.write(buffer.data()) != buffer.size()) {
                    errorSet = true;
                    future->setProgressValueAndText(0, tr("Cannot write an update file"));
                    return loop.quit();
                }
                if (!file.commit()) {
                    errorSet = true;
                    future->setProgressValueAndText(0, tr("Cannot commit an update file"));
                }
            } else {
                errorSet = true;
                future->setProgressValueAndText(0, tr("Download failed"));
            }
            loop.quit();
        });
        downloader.start();
        loop.exec();

        if (future->isCanceled() || errorSet)
            return 0;
    }

    future->reportResult(100);
    return 100;
}

void UpdateManager::onConnect()
{
    const UpdateSettings* settings = SystemSettings::updateSettings();
    if (settings->checkForUpdatesAutomatically && !s_isUpdateCheckRunning) {
        const QFileInfo localChecksums(ApplicationCore::updatesPath() + QLatin1String("/Checksums.cbor"));
        UpdateManager::startUpdateCheck(s_localChecksums.isEmpty()
                                        || !localChecksums.exists()
                                        || localChecksums.lastModified().daysTo(QDateTime::currentDateTime()) > 5);
    }
}

void UpdateManager::onDisconnect()
{
    s_remoteChecksums.clear();
    s_changelog.clear();
    if (s_isUpdateCheckRunning && s_localChecksumsWatcher.isFinished()) {
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(false);
    }
}

void UpdateManager::onLocalScanFinish()
{
    s_localChecksums = s_localChecksumsWatcher.future().result();
    s_isUpdateCheckRunning = false;

    if (!QDir(ApplicationCore::updatesPath()).mkpath(".")) {
        qWarning("WARNING: Cannot establish a new updates directory");
        emit updateCheckFinished(false);
        return;
    }
    QSaveFile file(ApplicationCore::updatesPath() + QLatin1String("/Checksums.cbor"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("WARNING: Cannot open local checksums file to save");
        emit updateCheckFinished(false);
        return;
    }
    file.write(s_localChecksums.toCborValue().toCbor());
    file.commit();

    emit updateCheckFinished(!s_remoteChecksums.isEmpty() && !s_changelog.isEmpty() && !s_localChecksums.isEmpty());
}

void UpdateManager::onChecksumsDownloaderResolved()
{
    if (s_checksumsDownloader.contentLength() > 0)
        s_checksumsBuffer.buffer().reserve(s_checksumsDownloader.contentLength());
    if (!s_checksumsBuffer.isOpen())
        s_checksumsBuffer.open(QIODevice::WriteOnly);
}

void UpdateManager::onChangelogDownloaderResolved()
{
    if (s_changelogDownloader.contentLength() > 0)
        s_changelogBuffer.buffer().reserve(s_changelogDownloader.contentLength());
    if (!s_changelogBuffer.isOpen())
        s_changelogBuffer.open(QIODevice::WriteOnly);
}

void UpdateManager::onChecksumsDownloaderReadyRead(int id)
{
    s_checksumsBuffer.seek(s_checksumsDownloader.head(id) + s_checksumsDownloader.pos(id));
    s_checksumsBuffer.write(s_checksumsDownloader.readAll(id));
}

void UpdateManager::onChangelogDownloaderReadyRead(int id)
{
    s_changelogBuffer.seek(s_changelogDownloader.head(id) + s_changelogDownloader.pos(id));
    s_changelogBuffer.write(s_changelogDownloader.readAll(id));
}

void UpdateManager::onChecksumsDownloaderFinished()
{
    if (s_changelogDownloader.isFinished() && s_changelogDownloader.isError())
        return;

    if (s_checksumsDownloader.isError() || s_checksumsDownloader.bytesReceived() <= 0) {
        handleDownloaderError();
        return;
    }

    if (s_checksumsBuffer.isOpen())
        s_checksumsBuffer.close();

    s_remoteChecksums = QCborValue::fromCbor(s_checksumsBuffer.data()).toMap();
    s_checksumsBuffer.buffer().clear();

    if (s_changelogDownloader.isFinished()) {
        if (s_localChecksums.isEmpty()) {
            if (s_localChecksumsWatcher.isFinished()) {
                s_localChecksumsWatcher.setFuture(Async::run(QThreadPool::globalInstance(),
                                                             &UpdateManager::generateUpdateChecksums,
                                                             topUpdateDir(), topUpdateDir()));
            }
        } else {
            s_isUpdateCheckRunning = false;
            emit updateCheckFinished(!s_remoteChecksums.isEmpty() && !s_changelog.isEmpty());
        }
    }
}

void UpdateManager::onChangelogDownloaderFinished()
{
    if (s_checksumsDownloader.isFinished() && s_checksumsDownloader.isError())
        return;

    if (s_changelogDownloader.isError() || s_changelogDownloader.bytesReceived() <= 0) {
        handleDownloaderError();
        return;
    }

    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();

    s_changelog = QString::fromUtf8(s_changelogBuffer.data());
    s_changelogBuffer.buffer().clear();

    if (s_checksumsDownloader.isFinished()) {
        if (s_localChecksums.isEmpty()) {
            if (s_localChecksumsWatcher.isFinished()) {
                s_localChecksumsWatcher.setFuture(Async::run(QThreadPool::globalInstance(),
                                                             &UpdateManager::generateUpdateChecksums,
                                                             topUpdateDir(), topUpdateDir()));
            }
        } else {
            s_isUpdateCheckRunning = false;
            emit updateCheckFinished(!s_remoteChecksums.isEmpty() && !s_changelog.isEmpty());
        }
    }
}

void UpdateManager::onUpdateCheckFinish(bool succeed)
{
    if (!succeed)
        return;

    s_downloadSize = 0;
    s_checksumsDiff.clear();

    foreach (const QCborValue& key, s_localChecksums.keys()) {
        if (!key.isUndefined() && !key.toString().isEmpty()) {
            const QCborMap& localVal = s_localChecksums.value(key).toMap();
            const QCborMap& remoteVal = s_remoteChecksums.value(key).toMap();
            const QByteArray& localHash = localVal.value(QStringLiteral("sha1")).toByteArray();
            const QByteArray& remoteHash = remoteVal.value(QStringLiteral("sha1")).toByteArray();
            if (localHash != remoteHash)
                s_checksumsDiff[key] = true; // Mark for removal
        }
    }

    foreach (const QCborValue& key, s_remoteChecksums.keys()) {
        if (!key.isUndefined() && !key.toString().isEmpty()) {
            const QCborMap& localVal = s_localChecksums.value(key).toMap();
            const QCborMap& remoteVal = s_remoteChecksums.value(key).toMap();
            const QByteArray& localHash = localVal.value(QStringLiteral("sha1")).toByteArray();
            const QByteArray& remoteHash = remoteVal.value(QStringLiteral("sha1")).toByteArray();
            if (localHash != remoteHash) {
                s_checksumsDiff[key] = false; // Mark for replacement or new
                s_downloadSize += remoteVal.value(QStringLiteral("size")).toInteger();
            }
        }
    }
}

void UpdateManager::onDownloadWatcherProgressValueChange(int progressValue)
{
    emit downloadProgress(progressValue);
}

void UpdateManager::onDownloadWatcherFinish()
{
    emit downloadFinished(s_downloadWatcher.isCanceled(), s_downloadWatcher.progressText());
}

void UpdateManager::handleDownloaderError()
{
    if (s_checksumsDownloader.isRunning())
        s_checksumsDownloader.abort();
    if (s_changelogDownloader.isRunning())
        s_changelogDownloader.abort();
    if (s_checksumsBuffer.isOpen())
        s_checksumsBuffer.close();
    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();
    s_checksumsBuffer.buffer().clear();
    s_changelogBuffer.buffer().clear();
    s_isUpdateCheckRunning = false;
    emit s_instance->updateCheckFinished(false);
}
