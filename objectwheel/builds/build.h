#ifndef BUILD_H
#define BUILD_H

#include <QCborMap>
#include <QTime>

class Build final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Build)

    friend class BuildModel;

public:
    explicit Build(const QCborMap& request, QObject* parent = nullptr);

    const QCborMap& request() const;
    QString status() const;
    QTime timeLeft() const;
    qreal speed() const;
    int totalBytes() const;
    int receivedBytes() const;

private:
    void setStatus(const QString& status);
    void setTimeLeft(const QTime& timeLeft);
    void setSpeed(qreal speed);
    void setTotalBytes(int totalBytes);
    void setReceivedBytes(int receivedBytes);

private:
    const QCborMap m_request;
    QString m_status;
    QTime m_timeLeft;
    qreal m_speed;
    int m_totalBytes;
    int m_receivedBytes;
};

#endif // BUILD_H
