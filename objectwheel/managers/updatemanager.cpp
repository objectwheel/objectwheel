#include <updatemanager.h>
#include <servermanager.h>
#include <async.h>
#include <systemsettings.h>
#include <updatesettings.h>
#include <applicationcore.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QFuture>

enum StatusCode {
    BadRequest,
    RequestSucceed,
};
Q_DECLARE_METATYPE(StatusCode)

UpdateManager* UpdateManager::s_instance = nullptr;
bool UpdateManager::s_isUpdateCheckRunning = false;
QCborMap UpdateManager::s_localMetaInfo;
QCborMap UpdateManager::s_remoteMetaInfo;
QString UpdateManager::s_changelog;
QFutureWatcher<QCborMap> UpdateManager::s_localMetaInfoWatcher;

UpdateManager::UpdateManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(ServerManager::instance(), &ServerManager::connected,
            this, &UpdateManager::onConnect, Qt::QueuedConnection);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &UpdateManager::onDisconnect, Qt::QueuedConnection);
    connect(ServerManager::instance(), &ServerManager::binaryMessageReceived,
            this, &UpdateManager::onServerResponse, Qt::QueuedConnection);
    connect(&s_localMetaInfoWatcher, &QFutureWatcher<QCborMap>::resultsReadyAt,
            this, &UpdateManager::onLocalScanFinish);
    do {
        QFile file(ApplicationCore::updatesPath() + QLatin1String("/Updates.meta"));
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
        s_localMetaInfo.clear();
        ServerManager::send(ServerManager::RequestUpdateMetaInfo, hostOS());
        s_isUpdateCheckRunning = true;
        emit instance()->updateCheckStarted();
    }
}

bool UpdateManager::isUpdateCheckRunning()
{
    return s_isUpdateCheckRunning;
}

QString UpdateManager::hostOS()
{
    // TODO: Make sure to handle different architectures
#if defined(Q_OS_MACOS)
    return QStringLiteral("macos");
#elif defined(Q_OS_WINDOWS)
    return QStringLiteral("windows");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("linux");
#endif
    return QString();
}

QDir UpdateManager::topDir()
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

QCborMap UpdateManager::generateCacheForDir(const QDir& dir)
{
    QCborMap cache;
    foreach (const QFileInfo& info, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
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
            cache.insert(topDir().relativeFilePath(info.absoluteFilePath()), QString::number(info.size())
                         + QLatin1Char('/')
                         + QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha1).toHex());
        }
    }
    return cache;
}

void UpdateManager::onConnect()
{
    const UpdateSettings* settings = SystemSettings::updateSettings();
    if (settings->checkForUpdatesAutomatically)
        UpdateManager::scheduleUpdateCheck(settings->lastUpdateCheckDate.daysTo(QDateTime::currentDateTime()) > 5);
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
    QFile file(ApplicationCore::updatesPath() + QLatin1String("/Updates.meta"));
    if (!file.open(QFile::WriteOnly)) {
        qWarning("Cannot open updates meta file to save");
        return;
    }
    file.write(s_localMetaInfo.toCborValue().toCbor());
    file.close();

    emit updateCheckFinished(!s_remoteMetaInfo.isEmpty());
}

void UpdateManager::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    if (command != ServerManager::ResponseUpdateMetaInfo)
        return;

    QCborArray array;
    StatusCode status;
    UtilityFunctions::pullCbor(data, command, status, array);

    if (status == RequestSucceed) {
        s_remoteMetaInfo.clear();
        s_changelog.clear();
        if (array.size() == 2) {
            s_remoteMetaInfo = array.first().toMap();
            s_changelog = array.last().toString();
        }
        if (s_localMetaInfo.isEmpty()) {
            if (s_localMetaInfoWatcher.isFinished()) {
                s_localMetaInfoWatcher.setFuture(Async::run(QThreadPool::globalInstance(),
                                                            &UpdateManager::generateCacheForDir, topDir()));
            }
        } else {
            s_isUpdateCheckRunning = false;
            emit updateCheckFinished(!s_remoteMetaInfo.isEmpty());
        }
    } else {
        qWarning("WARNING: Requesting update meta info failed.");
        s_isUpdateCheckRunning = false;
        emit updateCheckFinished(false);
    }
}
