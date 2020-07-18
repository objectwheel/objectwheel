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
#include <QFileInfo>
#include <QDir>
#include <QFuture>
#include <QTcpSocket>

UpdateManager* UpdateManager::s_instance = nullptr;
bool UpdateManager::s_isUpdateCheckRunning = false;
QBuffer UpdateManager::s_metaBuffer;
QBuffer UpdateManager::s_changelogBuffer;
FastDownloader UpdateManager::s_metaDownloader;
FastDownloader UpdateManager::s_changelogDownloader;
QCborMap UpdateManager::s_localMetaInfo;
QCborMap UpdateManager::s_remoteMetaInfo;
QCborMap UpdateManager::s_differences;
QString UpdateManager::s_changelog;
qint64 UpdateManager::s_downloadSize = 0;
QFutureWatcher<int> UpdateManager::s_downloadWatcher;
QFutureWatcher<QCborMap> UpdateManager::s_localMetaInfoWatcher;

static qint64 sizeFromValue(const QCborValue& value)
{
    const QStringList& list = value.toString().split(QLatin1Char('/'));
    if (list.size() == 2)
        return list.first().toLongLong();
    return 0;
}

static QString hashFromValue(const QCborValue& value)
{
    const QStringList& list = value.toString().split(QLatin1Char('/'));
    if (list.size() == 2)
        return list.last();
    return QString();
}

UpdateManager::UpdateManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(ServerManager::instance(), &ServerManager::connected,
            this, &UpdateManager::onConnect, Qt::QueuedConnection);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &UpdateManager::onDisconnect, Qt::QueuedConnection);
    connect(&s_localMetaInfoWatcher, &QFutureWatcher<QCborMap>::resultsReadyAt,
            this, &UpdateManager::onLocalScanFinish);
    connect(this, &UpdateManager::updateCheckFinished,
            this, &UpdateManager::onUpdateCheckFinish);
    connect(&s_metaDownloader, &FastDownloader::resolved,
            this, &UpdateManager::onMetaDownloaderResolved);
    connect(&s_metaDownloader, &FastDownloader::readyRead,
            this, &UpdateManager::onMetaDownloaderReadyRead);
    connect(&s_metaDownloader, qOverload<>(&FastDownloader::finished),
            this, &UpdateManager::onMetaDownloaderFinished);
    connect(&s_changelogDownloader, &FastDownloader::resolved,
            this, &UpdateManager::onChangelogDownloaderResolved);
    connect(&s_changelogDownloader, &FastDownloader::readyRead,
            this, &UpdateManager::onChangelogDownloaderReadyRead);
    connect(&s_changelogDownloader, qOverload<>(&FastDownloader::finished),
            this, &UpdateManager::onChangelogDownloaderFinished);

    s_metaDownloader.setUrl(QUrl(topUpdateRemotePath() + QStringLiteral("/update.meta")));
    s_metaDownloader.setNumberOfSimultaneousConnections(2);
    s_changelogDownloader.setUrl(QUrl(topUpdateRemotePath() + QStringLiteral("/changelog.html")));
    s_changelogDownloader.setNumberOfSimultaneousConnections(1);

    // WARNING: Remove an update artifact if any
    QFile::remove(QCoreApplication::applicationDirPath() + QLatin1String("/Updater.bak"));

    do {
        QFile file(ApplicationCore::updatesPath() + QLatin1String("/Local.meta"));
        if (!file.open(QFile::ReadOnly))
            break;
        s_localMetaInfo = QCborValue::fromCbor(file.readAll()).toMap();
    } while(false);
}

UpdateManager::~UpdateManager()
{
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

void UpdateManager::scheduleUpdateCheck(bool force)
{
    Q_ASSERT(ServerManager::isConnected());
    if (s_remoteMetaInfo.isEmpty() || force) {
        if (!s_metaDownloader.start()) {
            qWarning("Cannot start downloading for some reason");
            return;
        }
        if (!s_changelogDownloader.start()) {
            s_metaDownloader.abort();
            qWarning("Cannot start downloading for some reason");
            return;
        }
        if (force)
            s_localMetaInfo.clear();
        s_isUpdateCheckRunning = true;
        emit instance()->updateCheckStarted();
    }
}

bool UpdateManager::isUpdateCheckRunning()
{
    return s_isUpdateCheckRunning;
}

qint64 UpdateManager::downloadSize()
{
    return s_downloadSize;
}

void UpdateManager::update()
{
    // TODO
    s_downloadWatcher.setFuture(Async::run(QThreadPool::globalInstance(), &UpdateManager::download));

    QProcess::startDetached(QCoreApplication::applicationDirPath() + QLatin1String("/Updater"),
                            QStringList(ApplicationCore::updatesPath() + QLatin1String("/Diff.meta")));
    QCoreApplication::quit();
}

void UpdateManager::cancelUpdate()
{
    // TODO
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

QCborMap UpdateManager::generateCacheForDir(const QDir& dir)
{
    QCborMap cache;
    foreach (const QFileInfo& info, dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (info.isDir()) {
            const QCborMap& map = generateCacheForDir(info.absoluteFilePath());
            if (!map.isEmpty() && map.contains(QCborValue::Undefined))
                return map;
            foreach (const QCborValue& key, map.keys())
                cache.insert(key, map[key]);
        } else {
            QFile file(info.absoluteFilePath());
            if (!file.open(QFile::ReadOnly)) {
                qWarning() << "WARNING: Cannot open the file for reading, path:"<< info.absoluteFilePath();
                return QCborMap({{QCborValue::Undefined, QCborValue::Undefined}});
            }
            cache.insert(topUpdateDir().relativeFilePath(info.absoluteFilePath()), QString::number(info.size())
                         + QLatin1Char('/')
                         + QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1).toHex());
        }
    }
    return cache;
}

int UpdateManager::download(QFutureInterfaceBase* futureInterface)
{
    auto future = static_cast<QFutureInterface<int>*>(futureInterface);
    future->setProgressRange(0, 100);
    future->setProgressValue(0);

    qint64 downloadedSize = 0;
    const qint64 totalSize = s_downloadSize;

    QTcpSocket socket;
    socket.connectToHost(ServerManager::instance()->peerAddress(), 54544, QTcpSocket::ReadOnly);

    foreach (const QCborValue& key, s_differences.keys()) {
        if (future->isPaused())
            future->waitForResume();
        if (future->isCanceled())
            return 100;

        const QString& filePath = QDir::cleanPath(key.toString());
        const bool remove = s_differences.value(key).toBool(true);

        if (remove)
            continue;

        const QCborValue& value = s_remoteMetaInfo.value(key);
        const QString& fileHash = hashFromValue(value);
        const qint64 fileSize = sizeFromValue(value);

        downloadedSize += fileSize;
        future->setProgressValue(100 * qreal(downloadedSize) / totalSize);
    }

    future->reportResult(100);
    return 100;
}

void UpdateManager::onConnect()
{
    const UpdateSettings* settings = SystemSettings::updateSettings();
    if (settings->checkForUpdatesAutomatically) {
        const QFileInfo localInfo(ApplicationCore::updatesPath() + QLatin1String("/Local.meta"));
        UpdateManager::scheduleUpdateCheck(localInfo.lastModified().daysTo(QDateTime::currentDateTime()) > 4);
    }
}

void UpdateManager::onDisconnect()
{
    s_remoteMetaInfo.clear();
    s_changelog.clear();
    if (s_isUpdateCheckRunning && s_localMetaInfoWatcher.isFinished()) {
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(false);
    }
}

void UpdateManager::onLocalScanFinish()
{
    s_localMetaInfo = s_localMetaInfoWatcher.future().result();
    s_isUpdateCheckRunning = false;

    if (!QDir(ApplicationCore::updatesPath()).mkpath(".")) {
        qWarning("Cannot establish a new updates directory");
        return;
    }
    QFile file(ApplicationCore::updatesPath() + QLatin1String("/Local.meta"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("Cannot open updates meta file to save");
        return;
    }
    file.write(s_localMetaInfo.toCborValue().toCbor());
    file.close();

    emit updateCheckFinished(!s_remoteMetaInfo.isEmpty());
}

void UpdateManager::onMetaDownloaderResolved()
{
    if (s_metaDownloader.contentLength() > 0)
        s_metaBuffer.buffer().reserve(s_metaDownloader.contentLength());
    if (!s_metaBuffer.isOpen())
        s_metaBuffer.open(QIODevice::WriteOnly);
}

void UpdateManager::onChangelogDownloaderResolved()
{
    if (s_changelogDownloader.contentLength() > 0)
        s_changelogBuffer.buffer().reserve(s_changelogDownloader.contentLength());
    if (!s_changelogBuffer.isOpen())
        s_changelogBuffer.open(QIODevice::WriteOnly);
}

void UpdateManager::onMetaDownloaderReadyRead(int id)
{
    s_metaBuffer.seek(s_metaDownloader.head(id) + s_metaDownloader.pos(id));
    s_metaBuffer.write(s_metaDownloader.readAll(id));
}

void UpdateManager::onChangelogDownloaderReadyRead(int id)
{
    s_changelogBuffer.seek(s_changelogDownloader.head(id) + s_changelogDownloader.pos(id));
    s_changelogBuffer.write(s_changelogDownloader.readAll(id));
}

void UpdateManager::onMetaDownloaderFinished()
{
    if (s_metaBuffer.isOpen())
        s_metaBuffer.close();

    if (s_metaDownloader.isError() || s_metaDownloader.bytesReceived() <= 0) {
        qWarning("WARNING: Requesting update meta info failed.");
        s_metaBuffer.buffer().clear();
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(false);
    }

    s_remoteMetaInfo = QCborValue::fromCbor(s_metaBuffer.data()).toMap();
    s_metaBuffer.buffer().clear();

    if (s_localMetaInfo.isEmpty()) {
        if (s_localMetaInfoWatcher.isFinished()) {
            s_localMetaInfoWatcher.setFuture(Async::run(QThreadPool::globalInstance(),
                                                        &UpdateManager::generateCacheForDir, topUpdateDir()));
        }
    } else {
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(!s_remoteMetaInfo.isEmpty());
    }
}

void UpdateManager::onChangelogDownloaderFinished()
{
    if (s_changelogBuffer.isOpen())
        s_changelogBuffer.close();

    if (s_changelogDownloader.isError() || s_changelogDownloader.bytesReceived() <= 0) {
        qWarning("WARNING: Requesting update meta info failed.");
        s_changelogBuffer.buffer().clear();
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(false);
    }

    s_changelog = QCborValue::fromCbor(s_changelogBuffer.data()).toString();
    s_changelogBuffer.buffer().clear();

    if (s_localMetaInfo.isEmpty()) {
        if (s_localMetaInfoWatcher.isFinished()) {
            s_localMetaInfoWatcher.setFuture(Async::run(QThreadPool::globalInstance(),
                                                        &UpdateManager::generateCacheForDir, topUpdateDir()));
        }
    } else {
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(!s_remoteMetaInfo.isEmpty());
    }
}

void UpdateManager::onUpdateCheckFinish(bool succeed)
{
    if (!succeed)
        return;

    s_downloadSize = 0;
    s_differences.clear();

    foreach (const QCborValue& key, s_localMetaInfo.keys()) {
        const QCborValue& localVal = s_localMetaInfo.value(key);
        const QCborValue& remoteVal = s_remoteMetaInfo.value(key);
        const QString& localHash = hashFromValue(localVal);
        const QString& remoteHash = hashFromValue(remoteVal);
        if (QString::compare(localHash, remoteHash, Qt::CaseInsensitive) != 0)
            s_differences[key] = true; // Mark for removal
    }

    foreach (const QCborValue& key, s_remoteMetaInfo.keys()) {
        const QCborValue& localVal = s_localMetaInfo.value(key);
        const QCborValue& remoteVal = s_remoteMetaInfo.value(key);
        const QString& localHash = hashFromValue(localVal);
        const QString& remoteHash = hashFromValue(remoteVal);
        if (QString::compare(localHash, remoteHash, Qt::CaseInsensitive) != 0) {
            s_differences[key] = false; // Mark for replacement or new
            s_downloadSize += sizeFromValue(remoteVal);
        }
    }

    QFile file(ApplicationCore::updatesPath() + QLatin1String("/Diff.meta"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("Cannot open diff meta file to save");
        return;
    }
    file.write(s_differences.toCborValue().toCbor());
    file.close();
}
