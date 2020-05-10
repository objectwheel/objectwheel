#include <updatemanager.h>
#include <servermanager.h>

enum StatusCode {
    BadRequest,
    RequestSucceed,
};
Q_DECLARE_METATYPE(StatusCode)

UpdateManager* UpdateManager::s_instance = nullptr;
bool UpdateManager::s_updateCheckScheduled = false;
QCborMap UpdateManager::s_updateMetaInfo;

UpdateManager::UpdateManager(QObject* parent) : QObject(parent)
{
    s_instance = this;
    connect(ServerManager::instance(), &ServerManager::binaryMessageReceived,
            this, &UpdateManager::onServerResponse, Qt::QueuedConnection);
    connect(ServerManager::instance(), &ServerManager::connected,
            this, [] {
        if (s_updateCheckScheduled)
            scheduleUpdateCheck();
    });
}

UpdateManager::~UpdateManager()
{
    s_instance = nullptr;
}

QCborMap UpdateManager::updateMetaInfo()
{
    return s_updateMetaInfo;
}

UpdateManager* UpdateManager::instance()
{
    return s_instance;
}

void UpdateManager::scheduleUpdateCheck()
{
    if (!ServerManager::isConnected()) {
        s_updateCheckScheduled = true;
        return;
    }

    s_updateCheckScheduled = false;

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

void UpdateManager::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    if (command != ServerManager::ResponseUpdateMetaInfo)
        return;

    StatusCode status;
    UtilityFunctions::pullCbor(data, command, status, s_updateMetaInfo);

    if (status == RequestSucceed)
        emit metaInfoChanged();
    else
        qWarning("WARNING: Requesting update meta info failed.");
}

