#include <updatemanager.h>
#include <coreconstants.h>
#include <async.h>
#include <systemsettings.h>
#include <updatesettings.h>
#include <applicationcore.h>
#include <servermanager.h>

#include <QProcess>
#include <QDir>
#include <QSaveFile>
#include <QRandomGenerator>
#include <QTimer>

UpdateManager* UpdateManager::s_instance = nullptr;
bool UpdateManager::s_isUpdateCheckRunning = false;
qint64 UpdateManager::s_fileCount = 0;
qint64 UpdateManager::s_downloadSize = 0;
QDateTime UpdateManager::s_lastSuccessfulCheckup;
QString UpdateManager::s_changelog;
QCborMap UpdateManager::s_localChecksums;
QCborMap UpdateManager::s_remoteChecksums;
QCborMap UpdateManager::s_checksumsDiff;
QBuffer UpdateManager::s_changelogBuffer;
QBuffer UpdateManager::s_checksumsBuffer;
FastDownloader UpdateManager::s_changelogDownloader;
FastDownloader UpdateManager::s_checksumsDownloader;
QFutureWatcher<QCborMap> UpdateManager::s_localScanWatcher;
QFutureWatcher<QVariantMap> UpdateManager::s_downloadWatcher;

UpdateManager::UpdateManager(QObject* parent) : QObject(parent)
{
    s_instance = this;

    connect(ServerManager::instance(), &ServerManager::connected,
            this, &UpdateManager::onServerManagerConnected, Qt::QueuedConnection);
    connect(&s_changelogDownloader, &FastDownloader::resolved,
            this, &UpdateManager::onChangelogDownloaderResolved);
    connect(&s_checksumsDownloader, &FastDownloader::resolved,
            this, &UpdateManager::onChecksumsDownloaderResolved);
    connect(&s_changelogDownloader, &FastDownloader::readyRead,
            this, &UpdateManager::onChangelogDownloaderReadyRead);
    connect(&s_checksumsDownloader, &FastDownloader::readyRead,
            this, &UpdateManager::onChecksumsDownloaderReadyRead);
    connect(&s_changelogDownloader, qOverload<>(&FastDownloader::finished),
            this, &UpdateManager::onChangelogDownloaderFinished);
    connect(&s_checksumsDownloader, qOverload<>(&FastDownloader::finished),
            this, &UpdateManager::onChecksumsDownloaderFinished);
    connect(&s_localScanWatcher, &QFutureWatcher<QCborMap>::resultsReadyAt,
            this, &UpdateManager::onLocalScanFinished);
    connect(&s_downloadWatcher, &QFutureWatcher<QVariantMap>::resultReadyAt,
            this, &UpdateManager::onDownloadWatcherResultReadyAt);
    connect(&s_downloadWatcher, &QFutureWatcher<QVariantMap>::finished,
            this, &UpdateManager::onDownloadWatcherFinished);

    // WARNING: Clean up update artifacts from previous install if there is any
    QFile::remove(QCoreApplication::applicationDirPath() + QLatin1String("/Updater.bak"));

    s_checksumsDownloader.setNumberOfSimultaneousConnections(2);
    s_changelogDownloader.setNumberOfSimultaneousConnections(2);
    s_checksumsDownloader.setUrl(QUrl(remoteUpdateRootPath() + QStringLiteral("/checksums.cbor")));
    s_changelogDownloader.setUrl(QUrl(remoteUpdateRootPath() + QStringLiteral("/changelog.html")));

    QFile file(ApplicationCore::updatesPath() + QLatin1String("/Checksums.cbor"));
    if (file.open(QFile::ReadOnly))
        s_localChecksums = QCborValue::fromCbor(file.readAll()).toMap();
}

UpdateManager::~UpdateManager()
{
    if (s_isUpdateCheckRunning) {
        s_isUpdateCheckRunning = false;
        emit s_instance->updateCheckFinished(false);
    }

    if (s_checksumsBuffer.isOpen())
        s_checksumsBuffer.close();

    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();

    if (s_checksumsDownloader.isRunning())
        s_checksumsDownloader.abort();

    if (s_changelogDownloader.isRunning())
        s_changelogDownloader.abort();

    if (s_localScanWatcher.isRunning()) {
        s_localScanWatcher.cancel();
        s_localScanWatcher.waitForFinished();
    }

    if (s_downloadWatcher.isRunning()) {
        s_downloadWatcher.cancel();
        s_downloadWatcher.waitForFinished();
    }

    s_fileCount = 0;
    s_downloadSize = 0;
    s_changelog.clear();
    s_checksumsDiff.clear();
    s_localChecksums.clear();
    s_remoteChecksums.clear();
    s_checksumsBuffer.buffer().clear();
    s_changelogBuffer.buffer().clear();
    s_instance = nullptr;
}

UpdateManager* UpdateManager::instance()
{
    return s_instance;
}

qint64 UpdateManager::fileCount()
{
    return s_fileCount;
}

qint64 UpdateManager::downloadSize()
{
    return s_downloadSize;
}

QString UpdateManager::changelog()
{
    return s_changelog;
}

bool UpdateManager::isUpdateCheckRunning()
{
    return s_isUpdateCheckRunning;
}

bool UpdateManager::isDownloadRunning()
{
    return s_downloadWatcher.isRunning();
}

void UpdateManager::startUpdateCheck(bool forceLocalScan)
{
    if (s_isUpdateCheckRunning) {
        s_isUpdateCheckRunning = false;
        emit s_instance->updateCheckFinished(false);
    }

    if (s_checksumsBuffer.isOpen())
        s_checksumsBuffer.close();

    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();

    if (s_checksumsDownloader.isRunning())
        s_checksumsDownloader.abort();

    if (s_changelogDownloader.isRunning())
        s_changelogDownloader.abort();

    if (s_localScanWatcher.isRunning()) {
        s_localScanWatcher.cancel();
        s_localScanWatcher.waitForFinished();
    }

    if (s_downloadWatcher.isRunning()) {
        s_downloadWatcher.cancel();
        s_downloadWatcher.waitForFinished();
    }

    if (forceLocalScan)
        s_localChecksums.clear();

    s_fileCount = 0;
    s_downloadSize = 0;
    s_changelog.clear();
    s_checksumsDiff.clear();
    s_remoteChecksums.clear();
    s_checksumsBuffer.buffer().clear();
    s_changelogBuffer.buffer().clear();
    s_isUpdateCheckRunning = true;

    s_checksumsDownloader.start();
    s_changelogDownloader.start();

    emit instance()->updateCheckStarted();
}

void UpdateManager::download()
{
    if (s_isUpdateCheckRunning) {
        qWarning("WARNING: Wait until info download is finished");
        return;
    }

    if (s_downloadWatcher.isRunning()) {
        qWarning("WARNING: Wait until active download is finished");
        return;
    }

    if (s_fileCount <= 0) {
        qWarning("WARNING: Nothing to download, check updates first");
        return;
    }

    s_downloadWatcher.setFuture(Async::run<QVariantMap>(QThreadPool::globalInstance(),
                                                        &UpdateManager::doDownload));
}

void UpdateManager::cancelDownload()
{
    if (s_downloadWatcher.isFinished()) {
        qWarning("WARNING: Cannot abort an already finished download");
        return;
    }

    s_downloadWatcher.cancel();
}

void UpdateManager::install()
{
    if (s_isUpdateCheckRunning) {
        qWarning("WARNING: Wait until info download is finished");
        return;
    }

    if (s_downloadWatcher.isRunning()) {
        qWarning("WARNING: Wait until active download is finished");
        return;
    }

    if (s_fileCount <= 0) {
        qWarning("WARNING: Nothing to install, check updates first");
        return;
    }

    if (!QDir(ApplicationCore::updatesPath()).mkpath(".")) {
        qWarning("WARNING: Cannot establish a new updates directory");
        return;
    }
    const QByteArray& checksumsDiff = s_checksumsDiff.toCborValue().toCbor();
    QSaveFile file(ApplicationCore::updatesPath() + QLatin1String("/ChecksumsDiff.cbor"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("Cannot open checksums diff file to write");
        return;
    }
    if (file.write(checksumsDiff) != checksumsDiff.size()) {
        qWarning("Cannot write checksums diff file to the disk");
        return;
    }
    if (!file.commit()) {
        qWarning("Cannot commit checksums diff file to the disk");
        return;
    }

    QProcess process;
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    process.setProgram(QCoreApplication::applicationDirPath() + QLatin1String("/Updater"));
    process.setArguments(QStringList(ApplicationCore::updatesPath() + QLatin1String("/ChecksumsDiff.cbor")));
    process.startDetached();
    QTimer::singleShot(200, [] { QCoreApplication::quit(); });
}

void UpdateManager::onServerManagerConnected()
{
    if (!SystemSettings::updateSettings()->checkForUpdatesAutomatically)
        return;

    if (s_isUpdateCheckRunning)
        return;

    if (s_downloadWatcher.isRunning())
        return;

    int checkupSecs = s_lastSuccessfulCheckup.secsTo(QDateTime::currentDateTime());
    if (s_lastSuccessfulCheckup.isValid() && checkupSecs > 0 && checkupSecs < 7200)
        return;

    // If s_localChecksums is empty, we are going to scan anyway
    const QFileInfo checksumsInfo(ApplicationCore::updatesPath() + QLatin1String("/Checksums.cbor"));
    int modificationDays = checksumsInfo.lastModified().daysTo(QDateTime::currentDateTime());
    UpdateManager::startUpdateCheck(checksumsInfo.exists() && (modificationDays < 0 || modificationDays > 7));
}

void UpdateManager::onChangelogDownloaderResolved()
{
    if (s_changelogDownloader.contentLength() > 0)
        s_changelogBuffer.buffer().reserve(s_changelogDownloader.contentLength());
    if (!s_changelogBuffer.isOpen())
        s_changelogBuffer.open(QIODevice::WriteOnly);
}

void UpdateManager::onChecksumsDownloaderResolved()
{
    if (s_checksumsDownloader.contentLength() > 0)
        s_checksumsBuffer.buffer().reserve(s_checksumsDownloader.contentLength());
    if (!s_checksumsBuffer.isOpen())
        s_checksumsBuffer.open(QIODevice::WriteOnly);
}

void UpdateManager::onChangelogDownloaderReadyRead(int id)
{
    if (s_changelogBuffer.isOpen()) {
        s_changelogBuffer.seek(s_changelogDownloader.head(id) + s_changelogDownloader.pos(id));
        s_changelogBuffer.write(s_changelogDownloader.readAll(id));
    }
}

void UpdateManager::onChecksumsDownloaderReadyRead(int id)
{
    if (s_checksumsBuffer.isOpen()) {
        s_checksumsBuffer.seek(s_checksumsDownloader.head(id) + s_checksumsDownloader.pos(id));
        s_checksumsBuffer.write(s_checksumsDownloader.readAll(id));
    }
}

void UpdateManager::onChangelogDownloaderFinished()
{
    if (s_changelogDownloader.error() == QNetworkReply::OperationCanceledError)
        return;

    if (s_checksumsDownloader.isFinished() && s_checksumsDownloader.isError())
        return;

    if (s_changelogDownloader.isError() || s_changelogDownloader.bytesReceived() <= 0) {
        handleInfoDownloaderError();
        return;
    }

    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();

    s_changelog = QString::fromUtf8(s_changelogBuffer.data());
    s_changelogBuffer.buffer().clear();

    if (s_checksumsDownloader.isFinished()) {
        if (s_localChecksums.isEmpty()) {
            if (s_localScanWatcher.isFinished()) {
                s_localScanWatcher.setFuture(Async::run<QCborMap>(QThreadPool::globalInstance(),
                                                                  &UpdateManager::doLocalScan));
            }
        } else {
            s_isUpdateCheckRunning = false;
            const bool succeed = !s_changelog.isEmpty() && !s_remoteChecksums.isEmpty() && !s_localChecksums.isEmpty();
            if (succeed)
                handleDownloadInfoUpdate();
            emit updateCheckFinished(succeed);
        }
    }
}

void UpdateManager::onChecksumsDownloaderFinished()
{
    if (s_checksumsDownloader.error() == QNetworkReply::OperationCanceledError)
        return;

    if (s_changelogDownloader.isFinished() && s_changelogDownloader.isError())
        return;

    if (s_checksumsDownloader.isError() || s_checksumsDownloader.bytesReceived() <= 0) {
        handleInfoDownloaderError();
        return;
    }

    if (s_checksumsBuffer.isOpen())
        s_checksumsBuffer.close();

    s_remoteChecksums = QCborValue::fromCbor(s_checksumsBuffer.data()).toMap();
    s_checksumsBuffer.buffer().clear();

    if (s_changelogDownloader.isFinished()) {
        if (s_localChecksums.isEmpty()) {
            if (s_localScanWatcher.isFinished()) {
                s_localScanWatcher.setFuture(Async::run<QCborMap>(QThreadPool::globalInstance(),
                                                                  &UpdateManager::doLocalScan));
            }
        } else {
            s_isUpdateCheckRunning = false;
            const bool succeed = !s_changelog.isEmpty() && !s_remoteChecksums.isEmpty() && !s_localChecksums.isEmpty();
            if (succeed)
                handleDownloadInfoUpdate();
            emit updateCheckFinished(succeed);
        }
    }
}

void UpdateManager::onLocalScanFinished()
{
    if (s_localScanWatcher.isCanceled())
        return;

    s_isUpdateCheckRunning = false;

    if (s_localScanWatcher.future().resultCount() > 0)
        s_localChecksums = s_localScanWatcher.future().result();

    if (!s_localChecksums.isEmpty()) {
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
    }

    const bool succeed = !s_changelog.isEmpty() && !s_remoteChecksums.isEmpty() && !s_localChecksums.isEmpty();
    if (succeed)
        handleDownloadInfoUpdate();
    emit updateCheckFinished(succeed);
}

void UpdateManager::onDownloadWatcherResultReadyAt(int resultIndex)
{
    const QVariantMap& result = s_downloadWatcher.resultAt(resultIndex);
    if (result.value(QStringLiteral("errorString")).toString().isEmpty()) {
        emit downloadProgress(result.value(QStringLiteral("bytesPerSec")).toReal(),
                              result.value(QStringLiteral("bytesReceived")).toLongLong(),
                              result.value(QStringLiteral("fileIndex")).toLongLong(),
                              result.value(QStringLiteral("fileName")).toString());
    }
}

void UpdateManager::onDownloadWatcherFinished()
{
    QVariantMap result;
    int resultCount = s_downloadWatcher.future().resultCount();
    if (resultCount > 0)
        result = s_downloadWatcher.resultAt(resultCount - 1);
    emit downloadFinished(s_downloadWatcher.isCanceled(), result.value(QStringLiteral("errorString")).toString());
}

QString UpdateManager::localUpdateRootPath()
{
    // TODO: Handle other OSes
#if defined(Q_OS_MACOS)
    return QFileInfo(QCoreApplication::applicationDirPath() + QStringLiteral("/../..")).canonicalFilePath();
#elif defined(Q_OS_WINDOWS)
    return QCoreApplication::applicationDirPath();
#elif defined(Q_OS_LINUX)
    return QCoreApplication::applicationDirPath();
#else
    return QString();
#endif
}

QString UpdateManager::remoteUpdateRootPath()
{
#if defined(Q_OS_MACOS)
    return CoreConstants::UPDATE_URL + QStringLiteral("/macos-") + UtilityFunctions::buildCpuArchitecture();
#elif defined(Q_OS_WINDOWS)
    return CoreConstants::UPDATE_URL + QStringLiteral("/windows-") + UtilityFunctions::buildCpuArchitecture();
#elif defined(Q_OS_LINUX)
    return CoreConstants::UPDATE_URL + QStringLiteral("/linux-") + UtilityFunctions::buildCpuArchitecture();
#else
    return QString();
#endif
}

void UpdateManager::handleInfoDownloaderError()
{
    if (s_checksumsBuffer.isOpen())
        s_checksumsBuffer.close();
    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();
    if (s_checksumsDownloader.isRunning())
        s_checksumsDownloader.abort();
    if (s_changelogDownloader.isRunning())
        s_changelogDownloader.abort();
    s_checksumsBuffer.buffer().clear();
    s_changelogBuffer.buffer().clear();
    s_isUpdateCheckRunning = false;
    emit s_instance->updateCheckFinished(false);
}

void UpdateManager::handleDownloadInfoUpdate()
{
    s_fileCount = 0;
    s_downloadSize = 0;
    s_checksumsDiff.clear();
    s_lastSuccessfulCheckup = QDateTime::currentDateTime();

    foreach (const QCborValue& key, s_localChecksums.keys()) {
        if (!key.toString().isEmpty()) {
            const QCborMap& localVal = s_localChecksums.value(key).toMap();
            const QCborMap& remoteVal = s_remoteChecksums.value(key).toMap();
            const QByteArray& localHash = localVal.value(QStringLiteral("sha1")).toByteArray();
            const QByteArray& remoteHash = remoteVal.value(QStringLiteral("sha1")).toByteArray();
            if (localHash != remoteHash)
                s_checksumsDiff[key] = true; // Mark for removal
        }
    }

    foreach (const QCborValue& key, s_remoteChecksums.keys()) {
        if (!key.toString().isEmpty()) {
            const QCborMap& localVal = s_localChecksums.value(key).toMap();
            const QCborMap& remoteVal = s_remoteChecksums.value(key).toMap();
            const QByteArray& localHash = localVal.value(QStringLiteral("sha1")).toByteArray();
            const QByteArray& remoteHash = remoteVal.value(QStringLiteral("sha1")).toByteArray();
            if (localHash != remoteHash) {
                s_checksumsDiff[key] = false; // Mark for replacement or new
                s_downloadSize += remoteVal.value(QStringLiteral("size")).toInteger();
                s_fileCount++;
            }
        }
    }
}

void UpdateManager::doLocalScan(QFutureInterfaceBase* futureInterface)
{
    auto future = static_cast<QFutureInterface<QCborMap>*>(futureInterface);
    const QDir rootDir(localUpdateRootPath());
    QVector<QDir> dirs;
    dirs.reserve(2500);
    dirs.append(rootDir);
    QCborMap checksums;

    for (int i = 0; i < dirs.size(); ++i) {
        const QDir dir = dirs.at(i);
        foreach (const QFileInfo& info, dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot)) {
            const QString& absoluteFilePath = info.absoluteFilePath();
            if (future->isCanceled())
                return;
            if (info.isDir()) {
                dirs.append(absoluteFilePath);
            } else {
                QFile file(absoluteFilePath);
                if (!file.open(QFile::ReadOnly)) {
                    qWarning() << "WARNING: Cannot open the file for reading, path:"<< absoluteFilePath;
                    return;
                }
                QCborMap map;
                map.insert(QStringLiteral("size"), info.size());
                map.insert(QStringLiteral("sha1"), QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1));
                checksums.insert(rootDir.relativeFilePath(absoluteFilePath), map);
            }
        }
    }

    future->reportResult(checksums);
}

void UpdateManager::doDownload(QFutureInterfaceBase* futureInterface)
{
    auto future = static_cast<QFutureInterface<QVariantMap>*>(futureInterface);
    QFutureWatcher<QVariantMap> watcher;
    watcher.setFuture(future->future());

    QString errorString;
    qint64 fileIndex = 0;
    qint64 bytesReceived = 0;

    for (const QCborValue& key : s_checksumsDiff.keys()) {
        if (future->isCanceled() || !errorString.isEmpty()) {
            future->reportResult({{ QStringLiteral("errorString"), errorString }});
            return;
        }

        if (s_checksumsDiff.value(key).toBool(true))
            continue;

        fileIndex++;

        const QString& localPath = ApplicationCore::updatesPath() + QStringLiteral("/Downloads/") + key.toString();
        const QString& remotePath = remoteUpdateRootPath() + QStringLiteral("/content/") + key.toString();
        const QCborMap& remote = s_remoteChecksums.value(key).toMap();
        const QByteArray& fileHash = remote.value(QStringLiteral("sha1")).toByteArray();
        const qint64 fileSize = remote.value(QStringLiteral("size")).toInteger();

        if (QFileInfo::exists(localPath)) {
            QFile file(localPath);
            if (!file.open(QFile::ReadOnly)) {
                future->reportResult({{ QStringLiteral("errorString"), tr("Cannot read update file.") }});
                return;
            }
            if (fileHash == QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1)) {
                bytesReceived += fileSize;
                if (future->isProgressUpdateNeeded() || (bytesReceived >= s_downloadSize && fileIndex >= s_fileCount)) {
                    QVariantMap result;
                    result.insert(QStringLiteral("bytesReceived"), bytesReceived);
                    result.insert(QStringLiteral("fileIndex"), fileIndex);
                    result.insert(QStringLiteral("fileName"), key.toString());
                    future->reportResult(result);
                }
                continue;
            }
        }

        QEventLoop loop;
        QBuffer buffer;
        buffer.open(QBuffer::WriteOnly);
        FastDownloader downloader(QUrl{remotePath});
        struct Block { int size; QDateTime timestamp; };
        QVector<Block> recentBlocks;

        const auto calculateTransferRate = [&] (int chunkSize) -> QVariantMap
        {
            QVariantMap result;
            const int IDEAL_BLOCK_SIZE = fileSize / qMax(1, chunkSize) / 80;
            recentBlocks.append({chunkSize, QDateTime::currentDateTime()});

            if (recentBlocks.size() == 1)
                recentBlocks.append({chunkSize, QDateTime::currentDateTime().addMSecs(QRandomGenerator::global()->bounded(3, 50))});

            if (recentBlocks.size() > qBound(3, IDEAL_BLOCK_SIZE, 100))
                recentBlocks.removeFirst();

            int transferredBytes = -recentBlocks.first().size;
            int elapedMs = recentBlocks.first().timestamp.msecsTo(recentBlocks.last().timestamp);
            for (const Block& block : qAsConst(recentBlocks))
                transferredBytes += block.size;
            if (elapedMs == 0)
                elapedMs = QRandomGenerator::global()->bounded(3, 50);
            qreal bytesPerMs = qMax(1., transferredBytes / qreal(elapedMs));
            result.insert(QStringLiteral("bytesPerSec"), bytesPerMs * 1000);
            result.insert(QStringLiteral("bytesReceived"), bytesReceived);
            result.insert(QStringLiteral("fileIndex"), fileIndex);
            result.insert(QStringLiteral("fileName"), key.toString());
            return result;
        };

        connect(&watcher, &QFutureWatcher<QVariantMap>::canceled, &loop, &QEventLoop::quit);
        connect(&downloader, &FastDownloader::readyRead, [&] (int id) {
            const qint64 pos = downloader.head(id) + downloader.pos(id);
            const QByteArray& chunk = downloader.readAll(id);
            if (!buffer.seek(pos)) {
                errorString = tr("Cannot seek further in the buffer.");
                return loop.quit();
            }
            if (buffer.write(chunk) != chunk.size()) {
                errorString = tr("Cannot write more data into the buffer.");
                return loop.quit();
            }
            bytesReceived += chunk.size();
            if (future->isProgressUpdateNeeded() || (bytesReceived >= s_downloadSize && fileIndex >= s_fileCount))
                future->reportResult(calculateTransferRate(chunk.size()));
        });
        connect(&downloader, qOverload<int>(&FastDownloader::finished), [&] (int id) {
            if (downloader.isError()) {
                const QString& reason = downloader.errorString(id);
                if (errorString.isEmpty() && !reason.isEmpty())
                    errorString = tr("Network error occurred, reason: ") + reason;
            }
        });
        connect(&downloader, qOverload<>(&FastDownloader::finished), [&] {
            if (buffer.isOpen())
                buffer.close();
            if (!downloader.isError()) {
                if (fileHash != QCryptographicHash::hash(buffer.data(), QCryptographicHash::Sha1)) {
                    errorString = tr("Dowloaded file is corrupted, try again later.");
                    return loop.quit();
                }
                if (!QFileInfo(localPath).dir().mkpath(".")) {
                    errorString = tr("Cannot establish an update path.");
                    return loop.quit();
                }
                QSaveFile file(localPath);
                if (!file.open(QFile::WriteOnly)) {
                    errorString = tr("Cannot open an update file.");
                    return loop.quit();
                }
                if (file.write(buffer.data()) != buffer.size()) {
                    errorString = tr("Cannot write an update file.");
                    return loop.quit();
                }
                if (!file.setPermissions(file.permissions()
                                         | QSaveFile::ExeUser
                                         | QSaveFile::ExeOwner
                                         | QSaveFile::ExeGroup
                                         | QSaveFile::ExeOther)) {
                    qWarning("WARNING: Cannot set permissions of %s", localPath.toUtf8().constData());
                }
                if (!file.commit())
                    errorString = tr("Cannot commit an update file.");
            } else if (errorString.isEmpty()) {
                errorString = tr("Unknown network error occurred.");
            }
            loop.quit();
        });
        downloader.start();
        loop.exec();
    }

    if (future->isCanceled() || !errorString.isEmpty())
        future->reportResult({{ QStringLiteral("errorString"), errorString }});
}
