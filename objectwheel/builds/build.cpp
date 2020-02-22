#include <build.h>

Build::Build(const QCborMap& request, QObject* parent) : QObject(parent)
  , m_request(request)
  , m_speed(0)
  , m_totalBytes(0)
  , m_receivedBytes(0)
{
}

const QCborMap& Build::request() const
{
    return m_request;
}

QBuffer* Build::buffer()
{
    return &m_buffer;
}

QString Build::uid() const
{
    return m_uid;
}

void Build::setUid(const QString& uid)
{
    m_uid = uid;
}

QString Build::status() const
{
    return m_status;
}

void Build::setStatus(const QString& status)
{
    m_status = status;
}

QTime Build::timeLeft() const
{
    return m_timeLeft;
}

void Build::setTimeLeft(const QTime& timeLeft)
{
    m_timeLeft = timeLeft;
}

qreal Build::speed() const
{
    return m_speed;
}

void Build::setSpeed(qreal speed)
{
    m_speed = speed;
}

int Build::totalBytes() const
{
    return m_totalBytes;
}

void Build::setTotalBytes(int totalBytes)
{
    m_totalBytes = totalBytes;
}

int Build::receivedBytes() const
{
    return m_receivedBytes;
}

void Build::setReceivedBytes(int receivedBytes)
{
    m_receivedBytes = receivedBytes;
}
