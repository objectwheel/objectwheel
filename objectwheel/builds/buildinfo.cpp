#include <buildinfo.h>
#include <buildmodel.h>

#include <QFile>
#include <QStandardPaths>

static QString packageSuffixFromRequest(const QCborMap& request)
{
    if (request.value(QLatin1String("platform")).toString() == QLatin1String("android")) {
        if (request.value(QLatin1String("aab")).toBool(false))
            return QLatin1String(".aab");
        else
            return QLatin1String(".apk");
    }
    return QString();
}

QStringList BuildInfo::s_paths;

BuildInfo::BuildInfo(const QCborMap& request, QObject* parent) : QObject(parent)
  , m_request(request)
  , m_errorFlag(false)
  , m_speed(0)
  , m_totalBytes(0)
  , m_transferredBytes(0)
  , m_state(BuildModel::Uploading)
{
    const QString& suffix = packageSuffixFromRequest(request);
    const QString& baseName = request.value(QLatin1String("name")).toString();
    const QString& basePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (!suffix.isEmpty() && !baseName.isEmpty() && !basePath.isEmpty()) {
        QString fullPath = basePath + QLatin1Char('/') + baseName + suffix;
        if (!QFile::exists(fullPath)) {
            m_path = fullPath;
            s_paths.append(m_path.toLower());
        } else {
            int i = 1;
            forever {
                fullPath = basePath + QLatin1Char('/') + baseName + QString::number(i++) + suffix;
                if (!QFile::exists(fullPath) && !s_paths.contains(fullPath.toLower())) {
                    m_path = fullPath;
                    s_paths.append(m_path.toLower());
                    break;
                }
            }
        }
    }
}

BuildInfo::~BuildInfo()
{
    s_paths.removeOne(m_path.toLower());
}

QString BuildInfo::uid() const
{
    return m_uid;
}

void BuildInfo::setUid(const QString& uid)
{
    m_uid = uid;
}

const QCborMap& BuildInfo::request() const
{
    return m_request;
}

const QString& BuildInfo::path() const
{
    return m_path;
}

const QString& BuildInfo::details() const
{
    return m_details;
}

QBuffer* BuildInfo::buffer()
{
    return &m_buffer;
}

QList<BuildInfo::Block>& BuildInfo::recentBlocks()
{
    return m_recentBlocks;
}

bool BuildInfo::hasError() const
{
    return m_errorFlag;
}

void BuildInfo::setErrorFlag(bool errorFlag)
{
    m_errorFlag = errorFlag;
}

QString BuildInfo::status() const
{
    return m_status;
}

void BuildInfo::setStatus(const QString& status)
{
    if (status.contains(QLatin1Char('\n'))) {
        QTextStream stream(status.toUtf8());
        QString line;
        while (stream.readLineInto(&line)) {
            if (!line.isEmpty())
                m_status = line;
        }
        m_details.append(status);
    } else {
        m_status = status;
        m_details.append(status + QLatin1Char('\n'));
    }
}

QTime BuildInfo::timeLeft() const
{
    return m_timeLeft;
}

void BuildInfo::setTimeLeft(const QTime& timeLeft)
{
    m_timeLeft = timeLeft;
}

qreal BuildInfo::speed() const
{
    return m_speed;
}

void BuildInfo::setSpeed(qreal speed)
{
    m_speed = speed;
}

int BuildInfo::totalBytes() const
{
    return m_totalBytes;
}

void BuildInfo::setTotalBytes(int totalBytes)
{
    m_totalBytes = totalBytes;
}

int BuildInfo::transferredBytes() const
{
    return m_transferredBytes;
}

void BuildInfo::setTransferredBytes(int transferredBytes)
{
    m_transferredBytes = transferredBytes;
}

int BuildInfo::state() const
{
    return m_state;
}

void BuildInfo::setState(int state)
{
    m_state = state;
}
