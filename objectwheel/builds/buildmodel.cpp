#include <buildmodel.h>
#include <buildinfo.h>
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
    BuildStatus,
    MakeFailed,
    QmakeFailed,
    InvalidProjectFile,
    InvalidProjectSettings,
    Canceled,
    Timedout,
    //
    BuildData
};
Q_DECLARE_METATYPE(StatusCode)

BuildModel::BuildModel(QObject* parent) : QAbstractListModel(parent)
{
    connect(ServerManager::instance(), &ServerManager::binaryMessageReceived,
            this, &BuildModel::onServerResponse);
    connect(ServerManager::instance(), &ServerManager::bytesWritten,
            this, &BuildModel::onServerBytesWritten);
}

BuildModel::~BuildModel()
{
    qDeleteAll(m_buildInfos.cbegin(), m_buildInfos.cend());
}

void BuildModel::addBuildRequest(const QCborMap& request)
{
    Q_ASSERT(uploadingBuildInfo() == nullptr);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_buildInfos.append(new BuildInfo(request, this));
    endInsertRows();
    setData(indexFromBuildInfo(uploadingBuildInfo()), tr("Compressing the project..."), StatusRole);
    QTimer::singleShot(100, this, &BuildModel::start);
}

Qt::ItemFlags BuildModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);
    if (index.isValid())
        f &= ~Qt::ItemIsSelectable;
    return f;
}

int BuildModel::rowCount(const QModelIndex&) const
{
    return m_buildInfos.count();
}

QVariant BuildModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_buildInfos.count())
        return QVariant();

    const BuildInfo* buildInfo = m_buildInfos.value(index.row());

    switch (role) {
    case Qt::BackgroundRole: {
        QLinearGradient background(0, 0, 0, 1);
        background.setCoordinateMode(QGradient::ObjectMode);
        background.setColorAt(0, "#15ffffff");
        background.setColorAt(1, "#10000000");
        return QBrush(background);
    }
    case Qt::ForegroundRole:
        return QApplication::palette().text();
    case Qt::FontRole: {
        QFont font(QApplication::font());
        font.setWeight(QFont::Light);
        font.setPixelSize(11);
        return font;
    }
    case Qt::DecorationRole:
        return QImage::fromData(buildInfo->request().value(QLatin1String("icon")).toByteArray());
    case ButtonSize:
        return QSize(16, 16);
    case NameRole:
        return QFileInfo(buildInfo->path()).fileName();
    case PathRole:
        return buildInfo->path();
    case PlatformIconRole:
        return platformIcon(buildInfo->request().value(QLatin1String("platform")).toString());
    case VersionRole:
        return buildInfo->request().value(QLatin1String("versionName")).toString();
    case AbisRole: {
        QStringList abis;
        foreach (const QCborValue& abi, buildInfo->request().value(QLatin1String("abis")).toArray())
            abis.append(abi.toString());
        return abis.join(QLatin1String(", "));
    }
    case ErrorRole:
        return buildInfo->hasError();
    case StateRole:
        return buildInfo->state();
    case StatusRole:
        return buildInfo->status();
    case Qt::StatusTipRole:
        return buildInfo->statusTip();
    case SpeedRole:
        return buildInfo->speed();
    case TimeLeftRole:
        return buildInfo->timeLeft();
    case TotalBytesRole:
        return buildInfo->totalBytes();
    case TransferredBytesRole:
        return buildInfo->transferredBytes();
    default:
        break;
    }

    return QVariant();
}

bool BuildModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.row() < 0 || index.row() >= m_buildInfos.count())
        return false;

    BuildInfo* buildInfo = m_buildInfos.value(index.row());

    switch (role) {
    case StatusRole:
        buildInfo->addStatus(qvariant_cast<QString>(value));
        break;
    case StateRole:
        buildInfo->setState(qvariant_cast<int>(value));
        break;
    case ErrorRole:
        buildInfo->setErrorFlag(qvariant_cast<bool>(value));
        break;
    case SpeedRole:
        buildInfo->setSpeed(qvariant_cast<qreal>(value));
        break;
    case TimeLeftRole:
        buildInfo->setTimeLeft(qvariant_cast<QTime>(value));
        break;
    case TotalBytesRole:
        buildInfo->setTotalBytes(qvariant_cast<int>(value));
        break;
    case TransferredBytesRole:
        buildInfo->setTransferredBytes(qvariant_cast<int>(value));
        break;
    default:
        return false;
    }

    QVector<int> changedRoles { role };
    if (role == StatusRole)
        changedRoles.append(Qt::StatusTipRole);

    emit dataChanged(index, index, changedRoles);

    return true;
}

bool BuildModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        BuildInfo* buildInfo = m_buildInfos.takeAt(row + i);
        if (buildInfo->state() != Finished && ServerManager::isConnected())
            ServerManager::send(ServerManager::CancelCloudBuild, buildInfo->uid());
        delete buildInfo;
    }
    endRemoveRows();

    return true;
}

void BuildModel::clear()
{
    beginResetModel();
    for (int i = 0; i < m_buildInfos.size(); ++i) {
        BuildInfo* buildInfo = m_buildInfos.at(i);
        if (buildInfo->state() != Finished && ServerManager::isConnected())
            ServerManager::send(ServerManager::CancelCloudBuild, buildInfo->uid());
        delete buildInfo;
    }
    m_buildInfos.clear();
    endResetModel();
}

void BuildModel::start()
{
    BuildInfo* buildInfo = uploadingBuildInfo();
    const QModelIndex& index = indexFromBuildInfo(buildInfo);

    Q_ASSERT(index.isValid());
    Q_ASSERT(ServerManager::isConnected());

    QByteArray data;
    QString tmpFilePath;
    {
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            setData(index, tr("Failed to establish a temporary file"), StatusRole);
            return;
        }
        tmpFilePath = tempFile.fileName();
    }
    if (tmpFilePath.isEmpty() || ZipAsync::zipSync(ProjectManager::dir(), tmpFilePath) <= 0) {
        setData(index, tr("Failed to compress the project"), StatusRole);
        return;
    }
    QFile tempFile(tmpFilePath);
    if (!tempFile.open(QFile::ReadOnly)) {
        setData(index, tr("Failed to open a temporary file"), StatusRole);
        return;
    }
    data = tempFile.readAll();
    tempFile.close();
    tempFile.remove();

    int uploadSize = (int) ServerManager::send(ServerManager::RequestCloudBuild,
                                               UserManager::email(),
                                               UserManager::password(),
                                               buildInfo->request(), data);

    buildInfo->addStatus(tr("Uploading the project..."));
    buildInfo->setTotalBytes(uploadSize);
    emit dataChanged(index, index, { StatusRole, Qt::StatusTipRole, TotalBytesRole });
}

void BuildModel::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    if (command != ServerManager::ResponseCloudBuild)
        return;

    StatusCode status;
    QString uid;
    UtilityFunctions::pullCbor(data, command, status, uid);
    BuildInfo* buildInfo = buildInfoFromUid(uid);

    if (buildInfo == 0)
        buildInfo = uploadingBuildInfo();

    if (buildInfo == 0) {
        qWarning("WARNING: Cannot find build information corresponding to incoming data");
        return;
    }

    if (buildInfo->state() == Finished) {
        qWarning("WARNING: Data arrived for an already finished build");
        return;
    }

    QSet<int> changedRoles;
    const QModelIndex& index = indexFromBuildInfo(buildInfo);
    Q_ASSERT(index.isValid());

    if (buildInfo->state() == Uploading) {
        buildInfo->recentBlocks().clear();
        buildInfo->setSpeed(0);
        buildInfo->setTotalBytes(0);
        buildInfo->setTransferredBytes(0);
        buildInfo->setTimeLeft(QTime());
        buildInfo->setState(Downloading);
        changedRoles.unite({ SpeedRole, TotalBytesRole, TransferredBytesRole, TimeLeftRole, StateRole });
        emit uploadFinished(index);
    }

    switch (status) {
    case InternalError:
        buildInfo->addStatus(tr("Server failure"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case BadRequest:
        buildInfo->addStatus(tr("Invalid request"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case InvalidUserCredential:
        buildInfo->addStatus(tr("Invalid user credential"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case SimultaneousBuildLimitExceeded:
        buildInfo->addStatus(tr("Simultaneous build limit exceeded"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case RequestSucceed:
        Q_ASSERT(!uid.isEmpty());
        buildInfo->setUid(uid);
        buildInfo->addStatus(tr("Processing the request..."));
        changedRoles.unite({ StatusRole, Qt::StatusTipRole });
        break;

    case SequenceNumberChanged: {
        int sequenceNumber;
        UtilityFunctions::pullCbor(data, command, status, uid, sequenceNumber);
        buildInfo->addStatus(tr("You are %1th in the queue...").arg(sequenceNumber));
        changedRoles.unite({ StatusRole, Qt::StatusTipRole });
    } break;

    case BuildProcessStarted:
        buildInfo->addStatus(tr("Build process started..."));
        changedRoles.unite({ StatusRole, Qt::StatusTipRole });
        break;

    case BuildStatus: {
        QByteArray buildStatus;
        UtilityFunctions::pullCbor(data, command, status, uid, buildStatus);
        buildInfo->addStatus(buildStatus);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole });
    } break;

    case MakeFailed:
        buildInfo->addStatus(tr("MAKE process failed"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case QmakeFailed:
        buildInfo->addStatus(tr("QMAKE process failed"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case InvalidProjectFile:
        buildInfo->addStatus(tr("Invalid project file"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case InvalidProjectSettings:
        buildInfo->addStatus(tr("Invalid build configuration"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case Canceled:
        buildInfo->addStatus(tr("Operation cancelled"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case Timedout:
        buildInfo->addStatus(tr("Operation timedout"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case BuildData: {
        // Decode data
        bool isLastFrame;
        int totalBytes;
        QByteArray chunk;
        UtilityFunctions::pullCbor(data, command, status, uid, isLastFrame, totalBytes, chunk);

        // First frame
        if (!buildInfo->buffer()->isOpen()) {
            buildInfo->addStatus(tr("Downloading..."));
            buildInfo->setTotalBytes(totalBytes);
            buildInfo->buffer()->buffer().reserve(totalBytes);
            buildInfo->buffer()->open(QBuffer::WriteOnly);
            changedRoles.unite({ StatusRole, Qt::StatusTipRole, TotalBytesRole });
        }

        // Write data into the buffer
        buildInfo->buffer()->write(chunk);
        buildInfo->setTransferredBytes(buildInfo->buffer()->size());
        changedRoles.unite({ TransferredBytesRole });

        // Last frame
        if (isLastFrame) {
            buildInfo->buffer()->close();
            buildInfo->addStatus(tr("Done"));
            buildInfo->setState(Finished);
            changedRoles.unite({ StatusRole, Qt::StatusTipRole, StateRole });
            do {
                QFile file(index.data(PathRole).toString());
                if (!file.open(QFile::WriteOnly))
                    break;
                file.write(buildInfo->buffer()->data());
            } while (false);
            emit downloadFinished(index);
        }

        // Calculate speed and timeLeft
        BuildInfo::Block block;
        block.size = chunk.size();
        block.timestamp = QTime::currentTime();
        buildInfo->recentBlocks().append(block);
        if (buildInfo->recentBlocks().size() > 10)
            buildInfo->recentBlocks().removeFirst();
        //
        if (buildInfo->recentBlocks().size() > 1) {
            int transferredBytes = -buildInfo->recentBlocks().first().size;
            int elapedMs = buildInfo->recentBlocks().first().timestamp.msecsTo(buildInfo->recentBlocks().last().timestamp);
            for (const BuildInfo::Block& block : qAsConst(buildInfo->recentBlocks()))
                transferredBytes += block.size;
            qreal bytesPerMs = transferredBytes / qreal(elapedMs);
            buildInfo->setSpeed(bytesPerMs * 1000);
            changedRoles.unite({ SpeedRole });
            //
            int bytesLeft = buildInfo->totalBytes() - buildInfo->transferredBytes();
            qreal msLeft = bytesLeft / bytesPerMs;
            buildInfo->setTimeLeft(QTime(0, 0).addMSecs(msLeft));
            changedRoles.unite({ TimeLeftRole });
        }
    } break;

    default:
        break;
    }

    emit dataChanged(index, index, QVector<int>(changedRoles.cbegin(), changedRoles.cend()));
}

void BuildModel::onServerBytesWritten(qint64 bytes)
{
    if (BuildInfo* buildInfo = uploadingBuildInfo()) {
        QSet<int> changedRoles;
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());

        buildInfo->setTransferredBytes(buildInfo->transferredBytes() + int(bytes));
        changedRoles.unite({ TransferredBytesRole });

        // Calculate speed and timeLeft
        BuildInfo::Block block;
        block.size = int(bytes);
        block.timestamp = QTime::currentTime();
        buildInfo->recentBlocks().append(block);
        if (buildInfo->recentBlocks().size() > 10)
            buildInfo->recentBlocks().removeFirst();
        //
        if (buildInfo->recentBlocks().size() > 1) {
            int transferredBytes = -buildInfo->recentBlocks().first().size;
            int elapedMs = buildInfo->recentBlocks().first().timestamp.msecsTo(buildInfo->recentBlocks().last().timestamp);
            for (const BuildInfo::Block& block : qAsConst(buildInfo->recentBlocks()))
                transferredBytes += block.size;
            qreal bytesPerMs = transferredBytes / qreal(elapedMs);
            buildInfo->setSpeed(bytesPerMs * 1000);
            changedRoles.unite({ SpeedRole });
            //
            int bytesLeft = buildInfo->totalBytes() - buildInfo->transferredBytes();
            qreal msLeft = bytesLeft / bytesPerMs;
            buildInfo->setTimeLeft(QTime(0, 0).addMSecs(msLeft));
            changedRoles.unite({ TimeLeftRole });
        }

        emit dataChanged(index, index, QVector<int>(changedRoles.cbegin(), changedRoles.cend()));
    }
}

QIcon BuildModel::platformIcon(const QString& rawPlatformName) const
{
    return QIcon(QLatin1String(":/images/builds/%1.svg").arg(rawPlatformName));
}

BuildInfo* BuildModel::uploadingBuildInfo() const
{
    for (BuildInfo* buildInfo : qAsConst(m_buildInfos)) {
        if (buildInfo->state() == Uploading)
            return buildInfo;
    }
    return nullptr;
}

BuildInfo* BuildModel::buildInfoFromUid(const QString& uid)
{
    if (uid.isEmpty())
        return nullptr;
    for (BuildInfo* buildInfo : qAsConst(m_buildInfos)) {
        if (buildInfo->uid() == uid)
            return buildInfo;
    }
    return nullptr;
}

QModelIndex BuildModel::indexFromBuildInfo(const BuildInfo* buildInfo) const
{
    int row = m_buildInfos.indexOf(const_cast<BuildInfo*>(buildInfo));
    if (row < 0)
        return QModelIndex();
    return index(row);
}
