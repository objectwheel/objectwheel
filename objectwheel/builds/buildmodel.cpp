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
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_buildInfos.append(new BuildInfo(request, tr("Compressing the project..."), this));
    endInsertRows();
    QTimer::singleShot(100, this, &BuildModel::start);
}

int BuildModel::rowCount(const QModelIndex&) const
{
    return m_buildInfos.count();
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
    case Qt::ToolTipRole:
    case Qt::WhatsThisRole:
    case Qt::StatusTipRole:
        return buildInfo->status();
    case StateRole:
        return buildInfo->state();
    case ErrorRole:
        return buildInfo->hasError();
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
    case Qt::ToolTipRole:
    case Qt::WhatsThisRole:
    case Qt::StatusTipRole:
        buildInfo->setStatus(qvariant_cast<QString>(value));
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
    emit dataChanged(index, index, { role });
    return true;
}

bool BuildModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        BuildInfo* buildInfo = m_buildInfos.takeAt(row + i);
        if (buildInfo->state() != Finished && ServerManager::instance() && ServerManager::isConnected())
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
        if (buildInfo->state() != Finished && ServerManager::instance() && ServerManager::isConnected())
            ServerManager::send(ServerManager::CancelCloudBuild, buildInfo->uid());
        delete buildInfo;
    }
    m_buildInfos.clear();
    endResetModel();
}

void BuildModel::start()
{
    Q_ASSERT(ServerManager::instance() && ServerManager::isConnected());
    Q_ASSERT(!m_buildInfos.isEmpty());
    BuildInfo* buildInfo = m_buildInfos.last();
    const QModelIndex& index = BuildModel::index(m_buildInfos.size() - 1);
    Q_ASSERT(index.isValid());

    QByteArray data;
    {
        QString tmpFilePath;
        {
            QTemporaryFile tempFile;
            if (!tempFile.open()) {
                buildInfo->setStatus(tr("Failed to establish temporary file"));
                emit dataChanged(index, index);
                return;
            }
            tmpFilePath = tempFile.fileName();
        }
        if (tmpFilePath.isEmpty() || ZipAsync::zipSync(ProjectManager::dir(), tmpFilePath) <= 0) {
            buildInfo->setStatus(tr("Failed to compress the project"));
            emit dataChanged(index, index);
            return;
        }
        QFile tempFile(tmpFilePath);
        if (!tempFile.open(QFile::ReadOnly)) {
            buildInfo->setStatus(tr("Failed to open temporary file"));
            emit dataChanged(index, index);
            return;
        }
        data = tempFile.readAll();
        tempFile.close();
        tempFile.remove();
    }

    int uploadSize = (int) ServerManager::send(ServerManager::RequestCloudBuild,
                                               UserManager::email(),
                                               UserManager::password(),
                                               buildInfo->request(), data);

    buildInfo->setStatus(tr("Uploading the project..."));
    buildInfo->setTotalBytes(uploadSize);
    emit dataChanged(index, index);
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

    if (buildInfo == 0)
        return;

    if (buildInfo->state() == Finished)
        return;

    int row = m_buildInfos.indexOf(buildInfo);
    if (row < 0)
        return;
    const QModelIndex& index = BuildModel::index(row);
    Q_ASSERT(index.isValid());

    if (buildInfo->state() == Uploading) {
        buildInfo->setSpeed(0);
        buildInfo->setTotalBytes(0);
        buildInfo->setTransferredBytes(0);
        buildInfo->setTimeLeft(QTime());
        buildInfo->recentBlocks().clear();
        buildInfo->setState(Downloading);
        emit uploadFinished(index);
    }

    switch (status) {
    case InternalError:
        buildInfo->setStatus(tr("Server failed"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case BadRequest:
        buildInfo->setStatus(tr("Invalid request"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case InvalidUserCredential:
        buildInfo->setStatus(tr("Invalid user credential"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case SimultaneousBuildLimitExceeded:
        buildInfo->setStatus(tr("Simultaneous build limit exceeded"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case RequestSucceed:
        Q_ASSERT(!uid.isEmpty());
        buildInfo->setUid(uid);
        buildInfo->setStatus(tr("Processing the request..."));
        break;
    case SequenceNumberChanged: {
        int sequenceNumber;
        UtilityFunctions::pullCbor(data, command, status, uid, sequenceNumber);
        buildInfo->setStatus(tr("You are %1th in the queue...").arg(sequenceNumber));
    } break;
    case BuildProcessStarted:
        buildInfo->setStatus(tr("Build process started..."));
        break;
    case BuildStatus: {
        QByteArray progress;
        UtilityFunctions::pullCbor(data, command, status, uid, progress);
        buildInfo->setStatus(progress);
    } break;
    case MakeFailed:
        buildInfo->setStatus(tr("MAKE process failed"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case QmakeFailed:
        buildInfo->setStatus(tr("QMAKE process failed"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case InvalidProjectFile:
        buildInfo->setStatus(tr("Invalid project file"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case InvalidProjectSettings:
        buildInfo->setStatus(tr("Invalid build configuration"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case Canceled:
        buildInfo->setStatus(tr("Operation cancelled"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case Timedout:
        buildInfo->setStatus(tr("Operation timedout"));
        buildInfo->setErrorFlag(true);
        buildInfo->setState(Finished);
        break;
    case BuildData: {
        buildInfo->setStatus(tr("Downloading..."));

        // Decode data
        bool isLastFrame;
        int totalBytes;
        QByteArray chunk;
        UtilityFunctions::pullCbor(data, command, status, uid, isLastFrame, totalBytes, chunk);

        // Fetch and buffer data
        if (!buildInfo->buffer()->isOpen()) {
            buildInfo->setTotalBytes(totalBytes);
            buildInfo->buffer()->buffer().reserve(totalBytes);
            buildInfo->buffer()->open(QBuffer::WriteOnly);
        }
        buildInfo->buffer()->write(chunk);
        buildInfo->setTransferredBytes(buildInfo->buffer()->size());
        if (isLastFrame) {
            buildInfo->buffer()->close();
            buildInfo->setStatus(tr("Done"));
            buildInfo->setState(Finished);
            do {
                QFile file(index.data(PathRole).toString());
                if (!file.open(QFile::WriteOnly))
                    break;
                file.write(buildInfo->buffer()->data());
            } while (false);
            emit downloadFinished(index);
        }

        // Gather 'speed' and 'time left' information
        BuildInfo::Block block;
        block.size = chunk.size();
        block.timestamp = QTime::currentTime();
        buildInfo->recentBlocks().append(block);
        if (buildInfo->recentBlocks().size() > 10)
            buildInfo->recentBlocks().removeFirst();

        if (buildInfo->recentBlocks().size() > 1) {
            // Calculate Speed
            int transferredBytes = -buildInfo->recentBlocks().first().size;
            int elapedMs = buildInfo->recentBlocks().first().timestamp.msecsTo(buildInfo->recentBlocks().last().timestamp);
            for (const BuildInfo::Block& block : qAsConst(buildInfo->recentBlocks()))
                transferredBytes += block.size;
            qreal bytesPerMs = transferredBytes / qreal(elapedMs);
            buildInfo->setSpeed(bytesPerMs * 1000);

            // Calculate Time Left
            int bytesLeft = buildInfo->totalBytes() - buildInfo->transferredBytes();
            qreal msLeft = bytesLeft / bytesPerMs;
            buildInfo->setTimeLeft(QTime(0, 0).addMSecs(msLeft));
        }
    } break;
    default:
        break;
    }

    emit dataChanged(index, index);
}

void BuildModel::onServerBytesWritten(qint64 bytes)
{
    if (!m_buildInfos.isEmpty()) {
        BuildInfo* buildInfo = m_buildInfos.last();
        if (buildInfo->state() == Uploading) {
            const QModelIndex& index = BuildModel::index(m_buildInfos.size() - 1);
            Q_ASSERT(index.isValid());
            buildInfo->setTransferredBytes(buildInfo->transferredBytes() + int(bytes));

            // Gather 'speed' and 'time left' information
            BuildInfo::Block block;
            block.size = int(bytes);
            block.timestamp = QTime::currentTime();
            buildInfo->recentBlocks().append(block);
            if (buildInfo->recentBlocks().size() > 10)
                buildInfo->recentBlocks().removeFirst();

            if (buildInfo->recentBlocks().size() > 1) {
                // Calculate Speed
                int transferredBytes = -buildInfo->recentBlocks().first().size;
                int elapedMs = buildInfo->recentBlocks().first().timestamp.msecsTo(buildInfo->recentBlocks().last().timestamp);
                for (const BuildInfo::Block& block : qAsConst(buildInfo->recentBlocks()))
                    transferredBytes += block.size;
                qreal bytesPerMs = transferredBytes / qreal(elapedMs);
                buildInfo->setSpeed(bytesPerMs * 1000);

                // Calculate Time Left
                int bytesLeft = buildInfo->totalBytes() - buildInfo->transferredBytes();
                qreal msLeft = bytesLeft / bytesPerMs;
                buildInfo->setTimeLeft(QTime(0, 0).addMSecs(msLeft));
            }

            emit dataChanged(index, index);
        }
    }
}

QIcon BuildModel::platformIcon(const QString& rawPlatformName) const
{
    return QIcon(QLatin1String(":/images/builds/%1.svg").arg(rawPlatformName));
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

BuildInfo* BuildModel::uploadingBuildInfo() const
{
    for (BuildInfo* buildInfo : qAsConst(m_buildInfos)) {
        if (buildInfo->state() == Uploading)
            return buildInfo;
    }
    return nullptr;
}
