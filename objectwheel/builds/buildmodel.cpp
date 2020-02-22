#include <buildmodel.h>
#include <build.h>
#include <servermanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <zipasync.h>

#include <QCborMap>
#include <QTemporaryFile>
#include <QTimer>
#include <QIcon>
#include <QApplication>

enum StatusCode {
    InternalError,
    //
    BadRequest,
    InvalidUserCredential,
    SimultaneousBuildLimitExceeded,
    RequestSucceed,
    //
    SequenceNumberChanged,
    BuildProcessStarted,
    //
    BuildProgress,
    MakeFailed,
    QmakeFailed,
    InvalidProjectFile,
    InvalidProjectSettings,
    Canceled,
    Timedout,
    //
    BuildSucceed
};
Q_DECLARE_METATYPE(StatusCode)

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

Qt::ItemFlags BuildModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);
    if (index.isValid())
        f &= ~Qt::ItemIsSelectable;
    return f;
}

QVariant BuildModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_builds.count())
        return QVariant();

    const Build* build = m_builds.value(index.row());
    switch (role) {
    case Qt::BackgroundRole:
        return QApplication::palette().window();
    case Qt::ForegroundRole:
        return QApplication::palette().text();
    case Qt::FontRole: {
        QFont font(QApplication::font());
        font.setPixelSize(10);
        return font;
    }
    case Qt::SizeHintRole:
        return QSize(0, 12 * 4 + 2 * 3 + 7 * 2);
    case Qt::DecorationRole:
        return QImage::fromData(build->request().value(QLatin1String("icon")).toByteArray());
    case ButtonSize:
        return QSize(16, 16);
    case NameRole:
        return build->request().value(QLatin1String("name")).toString() + packageSuffixFromRequest(build->request());
    case PlatformIconRole:
        return platformIcon(build->request().value(QLatin1String("platform")).toString());
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
    case TotalBytesRole:
        return build->totalBytes();
    case ReceivedBytesRole:
        return build->receivedBytes();
    default:
        break;
    }
    return QVariant();
}

void BuildModel::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    if (command == ServerManager::ResponseCloudBuild) {
        StatusCode status;
        QString uid;
        UtilityFunctions::pullCbor(data, command, status, uid);
        Build* build = buildFromUid(uid);
        switch (status) {
        case InternalError:
            m_builds.last()->setStatus(tr("Internal Error"));
            break;
        case BadRequest:
            m_builds.last()->setStatus(tr("Bad Request"));
            break;
        case InvalidUserCredential:
            m_builds.last()->setStatus(tr("Invalid User Credential"));
            break;
        case SimultaneousBuildLimitExceeded:
            m_builds.last()->setStatus(tr("Simultaneous Build Limit Exceeded"));
            break;

        case RequestSucceed:
            Q_ASSERT(!uid.isEmpty());
            m_builds.last()->setUid(uid);
            m_builds.last()->setStatus(tr("Requesting build..."));
            break;

        case SequenceNumberChanged:
            if (build) {
                int sequenceNumber;
                UtilityFunctions::pullCbor(data, command, status, uid, sequenceNumber);
                build->setStatus(tr("You are %1th in the queue...").arg(sequenceNumber));
            } else {
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            } break;
        case BuildProcessStarted:
            if (build)
                build->setStatus(tr("Build process started..."));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case BuildProgress:
            if (build) {
                QByteArray progress;
                UtilityFunctions::pullCbor(data, command, status, uid, progress);
                build->setStatus(progress);
            } else {
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            } break;
        case MakeFailed:
            if (build)
                build->setStatus(tr("Build process started..."));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case QmakeFailed:
            if (build)
                build->setStatus(tr("QMAKE Process Failed"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case InvalidProjectFile:
            if (build)
                build->setStatus(tr("Invalid Project File"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case InvalidProjectSettings:
            if (build)
                build->setStatus(tr("Invalid Project Settings"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case Canceled:
            if (build)
                build->setStatus(tr("Operation Cancelled"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case Timedout:
            if (build)
                build->setStatus(tr("Operation Timedout"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case BuildSucceed:
            if (build) {
                build->setStatus(tr("Downloading..."));

                bool isLastFrame;
                int totalBytes;
                QByteArray chunk;

                UtilityFunctions::pullCbor(data, command, status, uid, isLastFrame, totalBytes, chunk);

                if (!build->buffer()->isOpen()) {
                    build->setTotalBytes(totalBytes);
                    build->buffer()->buffer().reserve(totalBytes);
                    build->buffer()->open(QBuffer::WriteOnly);
                }

                build->buffer()->write(chunk);
                build->setReceivedBytes(build->buffer()->size());

                if (isLastFrame) {
                    build->buffer()->close();
                    build->setStatus(tr("Done"));
                }
            } else {
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            } break;
        default:
            break;
        }
        if (build == 0)
            build = m_builds.last();
        int row = m_builds.indexOf(build);
        if (row < 0)
            return;
        const QModelIndex& index = BuildModel::index(row);
        Q_ASSERT(index.isValid());
        emit dataChanged(index, index);
    }
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

    build->setStatus(tr("Connecting to the server...."));

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

    ServerManager::send(ServerManager::RequestCloudBuild,
                        UserManager::email(),
                        UserManager::password(),
                        build->request(), tempFile.readAll());
    tempFile.close();
    tempFile.remove();

    emit dataChanged(index, index);
}

QIcon BuildModel::platformIcon(const QString& rawPlatformName) const
{
    return QIcon(QLatin1String(":/images/builds/%1.svg").arg(rawPlatformName));
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

Build* BuildModel::buildFromUid(const QString& uid)
{
    if (uid.isEmpty())
        return nullptr;
    for (Build* build : qAsConst(m_builds)) {
        if (build->uid() == uid)
            return build;
    }
    return nullptr;
}
