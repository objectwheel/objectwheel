#ifndef BUILD_H
#define BUILD_H

#include <QCborMap>
#include <QTime>
#include <QBuffer>

class Build final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Build)

    friend class BuildModel;

    struct Block {
        int size;
        QTime timestamp;
    };

public:
    explicit Build(const QCborMap& request, QObject* parent = nullptr);

    const QCborMap& request() const;

    QBuffer* buffer();
    QList<Block>& recentBlocks();

    QString uid() const;
    void setUid(const QString& uid);

    QString status() const;
    void setStatus(const QString& status);

    QTime timeLeft() const;
    void setTimeLeft(const QTime& timeLeft);

    qreal speed() const;
    void setSpeed(qreal speed);

    int totalBytes() const;
    void setTotalBytes(int totalBytes);

    int receivedBytes() const;
    void setReceivedBytes(int receivedBytes);

private:
    const QCborMap m_request;
    QString m_uid;
    QString m_status;
    QTime m_timeLeft;
    qreal m_speed;
    int m_totalBytes;
    int m_receivedBytes;
    QBuffer m_buffer;
    QList<Block> m_recentBlocks;
};

#endif // BUILD_H
