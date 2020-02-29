#ifndef BUILDINFO_H
#define BUILDINFO_H

#include <QCborMap>
#include <QTime>
#include <QBuffer>

class BuildInfo final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildInfo)

    friend class BuildModel;

    struct Block {
        int size;
        QTime timestamp;
    };

public:
    enum State { Uploading, Downloading, Finished };

public:
    explicit BuildInfo(const QCborMap& request, const QString& status = QString(), QObject* parent = nullptr);

    const QCborMap& request() const;
    QBuffer* buffer();
    QList<Block>& recentBlocks();

public:
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

    int transferredBytes() const;
    void setTransferredBytes(int transferredBytes);

    State state() const;
    void setState(State state);

private:
    const QCborMap m_request;
    QBuffer m_buffer;
    QList<Block> m_recentBlocks;

private:
    QString m_uid;
    QString m_status;
    QTime m_timeLeft;
    qreal m_speed;
    int m_totalBytes;
    int m_transferredBytes;
    State m_state;
};

#endif // BUILDINFO_H
