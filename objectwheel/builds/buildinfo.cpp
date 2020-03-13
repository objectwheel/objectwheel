#include <buildinfo.h>
#include <buildmodel.h>

BuildInfo::BuildInfo(const QCborMap& request, const QString& status, QObject* parent) : QObject(parent)
  , m_request(request)
  , m_errorFlag(false)
  , m_status(status)
  , m_speed(0)
  , m_totalBytes(0)
  , m_transferredBytes(0)
  , m_state(BuildModel::Uploading)
{
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
