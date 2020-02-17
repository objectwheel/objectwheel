#include <build.h>

Build::Build(const QCborMap& request, QObject* parent) : QObject(parent)
  , m_request(request)
  , m_speed(-1)
  , m_totalDataSize(-1)
  , m_receivedDataSize(-1)
{
}

const QCborMap& Build::request() const
{
    return m_request;
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

int Build::totalDataSize() const
{
    return m_totalDataSize;
}

void Build::setTotalDataSize(int totalDataSize)
{
    m_totalDataSize = totalDataSize;
}

int Build::receivedDataSize() const
{
    return m_receivedDataSize;
}

void Build::setReceivedDataSize(int receivedDataSize)
{
    m_receivedDataSize = receivedDataSize;
}
