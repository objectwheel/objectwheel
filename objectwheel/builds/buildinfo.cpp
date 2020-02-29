#include <buildinfo.h>

BuildInfo::BuildInfo(const QCborMap& request, const QString& status, QObject* parent) : QObject(parent)
  , m_request(request)
  , m_status(status)
  , m_speed(0)
  , m_totalBytes(0)
  , m_transferredBytes(0)
  , m_state(Uploading)
{
}

const QCborMap& BuildInfo::request() const
{
    return m_request;
}

QBuffer* BuildInfo::buffer()
{
    return &m_buffer;
}

QList<BuildInfo::Block>& BuildInfo::recentBlocks()
{
    return m_recentBlocks;
}

QString BuildInfo::uid() const
{
    return m_uid;
}

void BuildInfo::setUid(const QString& uid)
{
    m_uid = uid;
}

QString BuildInfo::status() const
{
    return m_status;
}

void BuildInfo::setStatus(const QString& status)
{
    m_status = status;
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

BuildInfo::State BuildInfo::state() const
{
    return m_state;
}

void BuildInfo::setState(State state)
{
    m_state = state;
}
