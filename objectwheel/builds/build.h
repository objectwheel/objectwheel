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
    int totalDataSize() const;
    int receivedDataSize() const;

private:
    void setStatus(const QString& status);
    void setTimeLeft(const QTime& timeLeft);
    void setSpeed(qreal speed);
    void setTotalDataSize(int totalDataSize);
    void setReceivedDataSize(int receivedDataSize);

private:
    const QCborMap m_request;
    QString m_status;
    QTime m_timeLeft;
    qreal m_speed;
    int m_totalDataSize;
    int m_receivedDataSize;
};

#endif // BUILD_H
