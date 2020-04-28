#ifndef BUILDINFO_H
#define BUILDINFO_H

#include <QCborMap>
#include <QDateTime>

class BuildInfo final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildInfo)

    friend class BuildModel;

    struct Block {
        int size;
        QDateTime timestamp;
    };

public:
    explicit BuildInfo(const QCborMap& request, QObject* parent = nullptr);
    ~BuildInfo() override;

    QString identifier() const;
    const QCborMap& request() const;
    const QString& path() const;
    QList<Block>& recentBlocks();

    QString uid() const;
    void setUid(const QString& uid);

public:
    bool hasError() const;
    void setErrorFlag(bool errorFlag);

    QString statusTip() const;
    QString status() const;
    void addStatus(const QString& status);

    QString payloadUid() const;
    void setPayloadUid(const QString& payloadUid);

    QTime timeLeft() const;
    void setTimeLeft(const QTime& timeLeft);

    qreal speed() const;
    void setSpeed(qreal speed);

    int totalBytes() const;
    void setTotalBytes(int totalBytes);

    int transferredBytes() const;
    void setTransferredBytes(int transferredBytes);

    int state() const;
    void setState(int state);

private:
    static QStringList s_paths;
    const QString m_identifier;
    const QCborMap m_request;
    QString m_uid;
    QString m_path;
    QList<Block> m_recentBlocks;

private:
    bool m_errorFlag;
    QString m_status;
    QString m_statusTip;
    QString m_payloadUid;
    QTime m_timeLeft;
    qreal m_speed;
    int m_totalBytes;
    int m_transferredBytes;
    int m_state;
};

#endif // BUILDINFO_H
