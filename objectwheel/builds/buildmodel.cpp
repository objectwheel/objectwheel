#include <buildmodel.h>
#include <buildinfo.h>
#include <servermanager.h>
#include <usermanager.h>
#include <projectmanager.h>
#include <zipasync.h>
#include <payloadrelay.h>

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
    BuildSucceed
};
Q_DECLARE_METATYPE(StatusCode)

BuildModel::BuildModel(QObject* parent) : QAbstractListModel(parent)
  , m_payloadRelay(new PayloadRelay(ServerManager::Payload, ServerManager::ResponsePayload, this))
{
    connect(ServerManager::instance(), &ServerManager::binaryMessageReceived,
            this, &BuildModel::onServerResponse, Qt::QueuedConnection);
    //
    connect(m_payloadRelay, &PayloadRelay::bytesUploaded,
            this, &BuildModel::onPayloadBytesUploaded);
    connect(m_payloadRelay, &PayloadRelay::uploadFinished,
            this, &BuildModel::onPayloadUploadFinished);
    connect(m_payloadRelay, &PayloadRelay::uploadTimedout,
            this, &BuildModel::onPayloadUploadTimedout);
    //
    connect(m_payloadRelay, &PayloadRelay::bytesDownloaded,
            this, &BuildModel::onPayloadBytesDownloaded);
    connect(m_payloadRelay, &PayloadRelay::downloadFinished,
            this, &BuildModel::onPayloadDownloadFinished);
    connect(m_payloadRelay, &PayloadRelay::downloadTimedout,
            this, &BuildModel::onPayloadDownloadTimedout);
}

BuildModel::~BuildModel()
{
    qDeleteAll(m_buildInfos.cbegin(), m_buildInfos.cend());
}

void BuildModel::addBuildRequest(const QCborMap& request)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    auto buildInfo = new BuildInfo(request, this);
    m_buildInfos.append(buildInfo);
    endInsertRows();
    setData(indexFromBuildInfo(buildInfo), tr("Compressing the project..."), StatusRole);
    QTimer::singleShot(100, this, [=] {
        if (m_buildInfos.contains(buildInfo)
                && buildInfo->state() == BuildModel::Uploading) {
            start(buildInfo);
        }
    });
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
        m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        m_payloadRelay->cancelDownload(buildInfo->payloadUid());
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
        m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        m_payloadRelay->cancelDownload(buildInfo->payloadUid());
        if (buildInfo->state() != Finished && ServerManager::isConnected())
            ServerManager::send(ServerManager::CancelCloudBuild, buildInfo->uid());
        delete buildInfo;
    }
    m_buildInfos.clear();
    endResetModel();
}

void BuildModel::start(BuildInfo* buildInfo)
{
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

    const QByteArray& payload = UtilityFunctions::pushCbor(buildInfo->request(), data);
    const QString& payloadUid = m_payloadRelay->scheduleUpload(ServerManager::instance(), payload);

    ServerManager::send(ServerManager::RequestCloudBuild,
                        UserManager::email(),
                        UserManager::password(),
                        payloadUid);
    ServerManager::instance()->flush();

    buildInfo->setPayloadUid(payloadUid);
    buildInfo->setTotalBytes(payload.size());
    buildInfo->addStatus(tr("Uploading the project..."));

    emit dataChanged(index, index, { StatusRole, Qt::StatusTipRole, TotalBytesRole });
}

void BuildModel::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    if (command != ServerManager::ResponseCloudBuild)
        return;

    // NOTE: Beware uid represents the payloadUid until we get a RequestSucceed
    // response from the server; Other commands which occurs before that should
    // take it into consideration.

    StatusCode status;
    QString uid;
    UtilityFunctions::pullCbor(data, command, status, uid);
    BuildInfo* buildInfo = buildInfoFromUid(uid);

    if (buildInfo == 0)
        buildInfo = buildInfoFromPayloadUid(uid);

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

    switch (status) {
    case InternalError:
        buildInfo->addStatus(tr("Server failure"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        if (buildInfo->state() == Uploading)
            m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        else
            m_payloadRelay->cancelDownload(buildInfo->payloadUid());
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case BadRequest:
        buildInfo->addStatus(tr("Invalid request"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case InvalidUserCredential:
        buildInfo->addStatus(tr("Invalid user credential"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case SimultaneousBuildLimitExceeded:
        buildInfo->addStatus(tr("Simultaneous build limit exceeded"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case RequestSucceed: {
        Q_ASSERT(!uid.isEmpty());
        QString payloadUid;
        UtilityFunctions::pullCbor(data, command, status, payloadUid, uid);
        buildInfo->setUid(uid);
        buildInfo->addStatus(tr("Processing the request..."));
        changedRoles.unite({ StatusRole, Qt::StatusTipRole });
    } break;

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
        if (buildInfo->state() == Uploading)
            m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        else
            m_payloadRelay->cancelDownload(buildInfo->payloadUid());
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case Timedout:
        buildInfo->addStatus(tr("Operation timedout"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        if (buildInfo->state() == Uploading)
            m_payloadRelay->cancelUpload(buildInfo->payloadUid());
        else
            m_payloadRelay->cancelDownload(buildInfo->payloadUid());
        changedRoles.unite({ StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
        break;

    case BuildSucceed: {
        QString payloadUid;
        UtilityFunctions::pullCbor(data, command, status, uid, payloadUid);
        buildInfo->setPayloadUid(payloadUid);
        m_payloadRelay->registerDownload(ServerManager::instance(), payloadUid);
    } break;

    default:
        break;
    }

    emit dataChanged(index, index, QVector<int>(changedRoles.cbegin(), changedRoles.cend()));
}

void BuildModel::timerEvent(QTimerEvent* event)
{
    if (m_changeSignalTimer.timerId() == event->timerId()) {
        m_changeSignalTimer.stop();
        foreach (int row, m_changedRows.keys()) {
            if (row >= 0 && row < m_buildInfos.size()) {
                const QModelIndex& index = BuildModel::index(row);
                const QSet<int>& roles = m_changedRows[row];
                Q_ASSERT(index.isValid());
                emit dataChanged(index, index, QVector<int>(roles.cbegin(), roles.cend()));
            }
        }
        m_changedRows.clear();
    } else {
        QAbstractListModel::timerEvent(event);
    }
}

void BuildModel::emitDelayedDataChanged(const QModelIndex& index, const QVector<int>& roles)
{
    if (index.isValid()) {
        m_changedRows[index.row()].unite(QSet<int>(roles.cbegin(), roles.cend()));
        if (!m_changeSignalTimer.isActive())
            m_changeSignalTimer.start(80, this); // ~12 fps
    }
}

void BuildModel::onPayloadBytesUploaded(const QString& uid, int bytes)
{
    if (BuildInfo* buildInfo = buildInfoFromPayloadUid(uid)) {
        QSet<int> changedRoles;
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());

        buildInfo->setTransferredBytes(buildInfo->transferredBytes() + int(bytes));
        changedRoles.unite({ TransferredBytesRole });

        calculateTransferRate(buildInfo, bytes, changedRoles);
        emitDelayedDataChanged(index, QVector<int>(changedRoles.cbegin(), changedRoles.cend()));
    }
}

void BuildModel::onPayloadBytesDownloaded(const QString& payloadUid, const QByteArray& chunk, int totalBytes)
{
    if (BuildInfo* buildInfo = buildInfoFromPayloadUid(payloadUid)) {
        QSet<int> changedRoles;
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());

        if (buildInfo->totalBytes() == 0) {
            buildInfo->addStatus(tr("Downloading..."));
            buildInfo->setTotalBytes(totalBytes);
            changedRoles.unite({ StatusRole, Qt::StatusTipRole, TotalBytesRole });
        }

        buildInfo->setTransferredBytes(buildInfo->transferredBytes() + chunk.size());
        changedRoles.unite({ TransferredBytesRole });

        calculateTransferRate(buildInfo, chunk.size(), changedRoles);
        emitDelayedDataChanged(index, QVector<int>(changedRoles.cbegin(), changedRoles.cend()));
    }
}

void BuildModel::onPayloadUploadFinished(const QString& payloadUid)
{
    if (BuildInfo* buildInfo = buildInfoFromPayloadUid(payloadUid)) {
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());
        Q_ASSERT(buildInfo->state() == Uploading);

        buildInfo->addStatus(tr("Waiting the server to start..."));
        buildInfo->recentBlocks().clear();
        buildInfo->setSpeed(0);
        buildInfo->setTotalBytes(0);
        buildInfo->setTransferredBytes(0);
        buildInfo->setTimeLeft(QTime());
        buildInfo->setState(Downloading);

        emit uploadFinished(index);
        emit dataChanged(index, index, { StatusRole, Qt::StatusTipRole, SpeedRole, TotalBytesRole, TransferredBytesRole, TimeLeftRole, StateRole });
    }
}

void BuildModel::onPayloadDownloadFinished(const QString& payloadUid, const QByteArray& data)
{
    if (BuildInfo* buildInfo = buildInfoFromPayloadUid(payloadUid)) {
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());
        buildInfo->addStatus(tr("Done"));
        buildInfo->setState(Finished);
        do {
            const QString& filePath = index.data(PathRole).toString();
            if (QFile::exists(filePath)) {
                qWarning("BuildModel: Executable file already exists");
                break;
            }
            QFile file(filePath);
            if (!file.open(QFile::WriteOnly)) {
                qWarning("BuildModel: Cannot open executable file");
                break;
            }
            file.write(data);
        } while (false);
        emit downloadFinished(index);
        emit dataChanged(index, index, { StatusRole, Qt::StatusTipRole, StateRole });
    }
}

void BuildModel::onPayloadUploadTimedout(const QString& payloadUid)
{
    if (BuildInfo* buildInfo = buildInfoFromPayloadUid(payloadUid)) {
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());
        buildInfo->addStatus(tr("Upload timedout"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        emit dataChanged(index, index, { StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
    }
}

void BuildModel::onPayloadDownloadTimedout(const QString& payloadUid)
{
    if (BuildInfo* buildInfo = buildInfoFromPayloadUid(payloadUid)) {
        const QModelIndex& index = indexFromBuildInfo(buildInfo);
        Q_ASSERT(index.isValid());
        buildInfo->addStatus(tr("Download timedout"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        emit dataChanged(index, index, { StatusRole, Qt::StatusTipRole, ErrorRole, StateRole });
    }
}

QIcon BuildModel::platformIcon(const QString& rawPlatformName) const
{
    return QIcon(QLatin1String(":/images/builds/%1.svg").arg(rawPlatformName));
}

BuildInfo* BuildModel::buildInfoFromUid(const QString& uid)
{
    if (!uid.isEmpty()) {
        for (BuildInfo* buildInfo : qAsConst(m_buildInfos)) {
            if (buildInfo->uid() == uid)
                return buildInfo;
        }
    }
    return nullptr;
}

BuildInfo* BuildModel::buildInfoFromPayloadUid(const QString& payloadUid)
{
    if (!payloadUid.isEmpty()) {
        for (BuildInfo* buildInfo : qAsConst(m_buildInfos)) {
            if (buildInfo->payloadUid() == payloadUid)
                return buildInfo;
        }
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

void BuildModel::calculateTransferRate(BuildInfo* buildInfo, int chunkSize, QSet<int>& changedRoles) const
{
    BuildInfo::Block block;
    block.size = chunkSize;
    block.timestamp = QDateTime::currentDateTime();
    buildInfo->recentBlocks().append(block);
    if (buildInfo->recentBlocks().size() > 6)
        buildInfo->recentBlocks().removeFirst();

    if (buildInfo->recentBlocks().size() > 1) {
        int transferredBytes = -buildInfo->recentBlocks().first().size;
        int elapedMs = buildInfo->recentBlocks().first().timestamp.msecsTo(buildInfo->recentBlocks().last().timestamp);
        for (const BuildInfo::Block& block : qAsConst(buildInfo->recentBlocks()))
            transferredBytes += block.size;
        if (elapedMs == 0)
            elapedMs = 100;
        qreal bytesPerMs = qMax(1., transferredBytes / qreal(elapedMs));
        buildInfo->setSpeed(bytesPerMs * 1000);
        changedRoles.unite({ SpeedRole });

        int bytesLeft = buildInfo->totalBytes() - buildInfo->transferredBytes();
        qreal msLeft = bytesLeft / bytesPerMs;
        buildInfo->setTimeLeft(QTime(0, 0).addMSecs(msLeft));
        changedRoles.unite({ TimeLeftRole });
    }
}
