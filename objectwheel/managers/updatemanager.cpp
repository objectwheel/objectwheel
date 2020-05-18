#include <updatemanager.h>
#include <servermanager.h>
#include <async.h>
#include <generalsettings.h>
#include <updatesettings.h>

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

void UpdateManager::scheduleUpdateCheck()
{
    Q_ASSERT(ServerManager::isConnected());
    if (s_remoteMetaInfo.isEmpty())
        ServerManager::send(ServerManager::RequestUpdateMetaInfo, hostOS());
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
    if (GeneralSettings::updateSettings()->checkForUpdatesAutomatically
            && s_remoteMetaInfo.isEmpty()) {
        UpdateManager::scheduleUpdateCheck();
    }
}

void UpdateManager::onDisconnect()
{
    s_remoteMetaInfo.clear();
    s_changelog.clear();
}

void UpdateManager::onLocalScanFinish()
{
    s_localMetaInfo = s_localMetaInfoWatcher.future().result();
    if (!s_remoteMetaInfo.isEmpty())
        emit updateCheckFinished();
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
        if (array.size() == 2) {
            s_remoteMetaInfo = array.first().toMap();
            s_changelog = array.last().toString();
        }
        if (s_localMetaInfo.isEmpty() && s_localMetaInfoWatcher.isFinished()) {
            s_localMetaInfoWatcher.setFuture(Async::run(QThreadPool::globalInstance(),
                                                        &UpdateManager::generateCacheForDir, topDir()));
        }
    } else {
        qWarning("WARNING: Requesting update meta info failed.");
        emit updateCheckFinished();
    }
}
