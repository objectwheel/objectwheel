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
#include <QStandardPaths>

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
    case Qt::SizeHintRole:
        return QSize(0, 12 * 4 + 2 * 3 + 7 * 2);
    case Qt::DecorationRole:
        return QImage::fromData(buildInfo->request().value(QLatin1String("icon")).toByteArray());
    case ButtonSize:
        return QSize(16, 16);
    case NameRole: {
        return buildInfo->request().value(QLatin1String("name")).toString()
                + packageSuffixFromRequest(buildInfo->request());
    }
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
    case Qt::StatusTipRole:
    case Qt::WhatsThisRole:
    case StatusRole:
        return buildInfo->status();
    case SpeedRole:
        return buildInfo->speed();
    case TimeLeftRole:
        return buildInfo->timeLeft();
    case TotalBytesRole:
        return buildInfo->totalBytes();
    case ReceivedBytesRole:
        return buildInfo->receivedBytes();
    default:
        break;
    }
    return QVariant();
}

void BuildModel::clear()
{
    beginResetModel();
    qDeleteAll(m_buildInfos.begin(), m_buildInfos.end());
    m_buildInfos.clear();
    // FIXME: We also have to send "cancel" to the server,
    // and no matter what, server might still send data us
    // back so we have to ignore those too
    endResetModel();
}

void BuildModel::start()
{
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

    buildInfo->setStatus(tr("Uploading the project..."));
    emit dataChanged(index, index);

    ServerManager::send(ServerManager::RequestCloudBuild,
                        UserManager::email(),
                        UserManager::password(),
                        buildInfo->request(), data);
}

void BuildModel::onServerResponse(const QByteArray& data)
{
    ServerManager::ServerCommands command = ServerManager::Invalid;
    UtilityFunctions::pullCbor(data, command);

    if (command == ServerManager::ResponseCloudBuild) {
        StatusCode status;
        QString uid;
        UtilityFunctions::pullCbor(data, command, status, uid);
        BuildInfo* buildInfo = buildInfoFromUid(uid);
        switch (status) {
        case InternalError:
            m_buildInfos.last()->setStatus(tr("Internal Error"));
            break;
        case BadRequest:
            m_buildInfos.last()->setStatus(tr("Bad Request"));
            break;
        case InvalidUserCredential:
            m_buildInfos.last()->setStatus(tr("Invalid User Credential"));
            break;
        case SimultaneousBuildLimitExceeded:
            m_buildInfos.last()->setStatus(tr("Simultaneous Build Limit Exceeded"));
            break;

        case RequestSucceed:
            Q_ASSERT(!uid.isEmpty());
            m_buildInfos.last()->setUid(uid);
            m_buildInfos.last()->setStatus(tr("Requesting build..."));
            break;

        case SequenceNumberChanged:
            if (buildInfo) {
                int sequenceNumber;
                UtilityFunctions::pullCbor(data, command, status, uid, sequenceNumber);
                buildInfo->setStatus(tr("You are %1th in the queue...").arg(sequenceNumber));
            } else {
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            } break;
        case BuildProcessStarted:
            if (buildInfo)
                buildInfo->setStatus(tr("Build process started..."));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case BuildProgress:
            if (buildInfo) {
                QByteArray progress;
                UtilityFunctions::pullCbor(data, command, status, uid, progress);
                QTextStream stream(progress);
                QString line, final;
                while (stream.readLineInto(&line)) {
                    if (!line.isEmpty())
                        final = line;
                }
                buildInfo->setStatus(final);
            } else {
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            } break;
        case MakeFailed:
            if (buildInfo)
                buildInfo->setStatus(tr("Build process started..."));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case QmakeFailed:
            if (buildInfo)
                buildInfo->setStatus(tr("QMAKE Process Failed"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case InvalidProjectFile:
            if (buildInfo)
                buildInfo->setStatus(tr("Invalid Project File"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case InvalidProjectSettings:
            if (buildInfo)
                buildInfo->setStatus(tr("Invalid Project Settings"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case Canceled:
            if (buildInfo)
                buildInfo->setStatus(tr("Operation Cancelled"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case Timedout:
            if (buildInfo)
                buildInfo->setStatus(tr("Operation Timedout"));
            else
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            break;
        case BuildSucceed:
            if (buildInfo) {
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
                buildInfo->setReceivedBytes(buildInfo->buffer()->size());
                if (isLastFrame) {
                    buildInfo->buffer()->close();
                    buildInfo->setStatus(tr("Done"));
                    do {
                        int row = m_buildInfos.indexOf(buildInfo);
                        if (row < 0)
                            break;
                        const QModelIndex& index = BuildModel::index(row);
                        Q_ASSERT(index.isValid());
                        const QString& downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
                        QFile file(downloadPath + QLatin1Char('/') + index.data(NameRole).toString());
                        if (!file.open(QFile::WriteOnly))
                            break;
                        file.write(buildInfo->buffer()->data());
                    } while(false);
                }

                // Gather 'speed' and 'time left' information
                BuildInfo::Block block {
                    .size = chunk.size(),
                    .timestamp = QTime::currentTime()
                };
                buildInfo->recentBlocks().append(block);
                if (buildInfo->recentBlocks().size() > 10)
                    buildInfo->recentBlocks().removeFirst();

                if (buildInfo->recentBlocks().size() > 1) {
                    // Calculate Speed
                    int receivedBytes = -buildInfo->recentBlocks().first().size;
                    int elapedMs = buildInfo->recentBlocks().first().timestamp.msecsTo(buildInfo->recentBlocks().last().timestamp);
                    for (const BuildInfo::Block& block : qAsConst(buildInfo->recentBlocks()))
                        receivedBytes += block.size;
                    qreal bytesPerMs = receivedBytes / qreal(elapedMs);
                    buildInfo->setSpeed(bytesPerMs * 1000);

                    // Calculate Time Left
                    int bytesLeft = totalBytes - buildInfo->buffer()->size();
                    qreal msLeft = bytesLeft / bytesPerMs;
                    buildInfo->setTimeLeft(QTime(0, 0).addMSecs(msLeft));
                }
            } else {
                qWarning("WARNING: Cannot associate build uid to any existing builds");
            } break;
        default:
            break;
        }
        if (buildInfo == 0)
            buildInfo = m_buildInfos.last();
        int row = m_buildInfos.indexOf(buildInfo);
        if (row < 0)
            return;
        const QModelIndex& index = BuildModel::index(row);
        Q_ASSERT(index.isValid());
        emit dataChanged(index, index);
    }
}

void BuildModel::onServerBytesWritten(qint64 bytes)
{

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
