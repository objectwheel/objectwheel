#include <buildmodel.h>
#include <build.h>
#include <servermanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <zipasync.h>

#include <QCborMap>
#include <QTemporaryFile>
#include <QTimer>

enum StatusCode {
    SUCCEED,
    INTERNAL_ERROR,
    BAD_REQUEST,
    INVALID_USER_CREDENTIAL,
    SIMULTANEOUS_BUILD_LIMIT_EXCEEDED,
    MAKE_FAILED,
    QMAKE_FAILED,
    INVALID_PROJECT_FILE,
    INVALID_PROJECT_SETTINGS_FILE,
    CANCELED,
    TIMEDOUT,
};

BuildModel::BuildModel(QObject* parent) : QAbstractListModel(parent)
{
    connect(ServerManager::instance(), &ServerManager::binaryMessageReceived,
            this, &BuildModel::onServerResponse);
}

void BuildModel::addBuildRequest(const QCborMap& request)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    auto build = new Build(request, this);
    build->setStatus(tr("Compressing the project..."));
    m_builds.append(build);
    endInsertRows();
    scheduleConnection(build);
}

int BuildModel::rowCount(const QModelIndex&) const
{
    return m_builds.count();
}

QVariant BuildModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_builds.count())
        return QVariant();

    const Build* build = m_builds.value(index.row());
    switch (role) {
    case PlatformRole:
        return toPrettyPlatformName(build->request().value(QLatin1String("platform")).toString());
    case NameRole:
        return build->request().value(QLatin1String("name")).toString() + packageSuffixFromRequest(build->request());
    case IconRole:
        return QImage::fromData(build->request().value(QLatin1String("icon")).toByteArray());
    case VersionRole:
        return build->request().value(QLatin1String("versionName")).toString();
    case AbisRole: {
        QStringList abis;
        foreach (const QCborValue& abi, build->request().value(QLatin1String("abis")).toArray())
            abis.append(abi.toString());
        return abis.join(QLatin1String(", "));
    }
    case StatusRole:
        return build->status();
    case SpeedRole:
        return build->speed();
    case TimeLeftRole:
        return build->timeLeft();
    case TotalDataSizeRole:
        return build->totalDataSize();
    case ReceivedDataSizeRole:
        return build->receivedDataSize();
    default:
        break;
    }
    return QVariant();
}

void BuildModel::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    switch (command) {
//    case ServerManager::LoginSuccessful: {
//        QByteArray icon;
//        QDateTime regdate;
//        PlanManager::Plans plan;
//        QString first, last, country, company, title, phone;
//        UtilityFunctions::pullCbor(data, command, icon, regdate, plan, first, last, country, company, title, phone);

//        QVariantList userInfo;
//        userInfo.append(icon);
//        userInfo.append(regdate);
//        userInfo.append(plan);
//        userInfo.append(first);
//        userInfo.append(last);
//        userInfo.append(country);
//        userInfo.append(company);
//        userInfo.append(title);
//        userInfo.append(phone);
//        emit loginSuccessful(userInfo);
//    } break;
//    case ServerManager::LoginFailure:
//        emit loginFailure();
//        break;
    default:
        break;
    }

    //    emit dataChanged(bottom, top);
}

void BuildModel::scheduleConnection(Build* build)
{
    QTimer::singleShot(100, [=] { establishConnection(build); });
}

void BuildModel::establishConnection(Build* build)
{
    int row = m_builds.indexOf(build);
    if (row < 0)
        return;

    const QModelIndex& index = BuildModel::index(row);
    Q_ASSERT(index.isValid());

    build->setStatus(tr("Establishing connection to the server...."));

    QString tmpFilePath;
    {
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            qWarning("WARNING: Cannot open up a temporary file");
            build->setStatus(tr("An Internal Error Occurred"));
            emit dataChanged(index, index);
            return;
        }
        tmpFilePath = tempFile.fileName();
    }
    if (ZipAsync::zipSync(ProjectManager::dir(), tmpFilePath) == 0) {
        qWarning("WARNING: Cannot zip user project");
        build->setStatus(tr("An Internal Error Occurred"));
        emit dataChanged(index, index);
        return;
    }
    QFile tempFile(tmpFilePath);
    if (!tempFile.open(QFile::ReadOnly)) {
        qWarning("WARNING: Cannot open compressed project file");
        build->setStatus(tr("An Internal Error Occurred"));
        emit dataChanged(index, index);
        return;
    }

    ServerManager::send(ServerManager::RequestBuild,
                        UserManager::email(),
                        UserManager::password(),
                        build->request(), tempFile.readAll());
    tempFile.close();
    tempFile.remove();

    emit dataChanged(index, index);
}

QString BuildModel::toPrettyPlatformName(const QString& rawPlatformName) const
{
    if (rawPlatformName == QLatin1String("android"))
        return QLatin1String("Android");
    if (rawPlatformName == QLatin1String("ios"))
        return QLatin1String("iOS");
    if (rawPlatformName == QLatin1String("macos"))
        return QLatin1String("macOS");
    if (rawPlatformName == QLatin1String("windows"))
        return QLatin1String("Windows");
    if (rawPlatformName == QLatin1String("linux"))
        return QLatin1String("Linux");
    return QString();
}

QString BuildModel::packageSuffixFromRequest(const QCborMap& request) const
{
    if (request.value(QLatin1String("platform")).toString() == QLatin1String("android")) {
        if (request.value(QLatin1String("aab")).toBool(false))
            return QLatin1String(".aab");
        else
            return QLatin1String(".apk");
    }
    return QString();
}
