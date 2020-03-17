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
    explicit BuildInfo(const QCborMap& request, QObject* parent = nullptr);
    ~BuildInfo() override;

    const QCborMap& request() const;
    const QString& path() const;

    QBuffer* buffer();
    QList<Block>& recentBlocks();

    QString uid() const;
    void setUid(const QString& uid);

public:
    bool hasError() const;
    void setErrorFlag(bool errorFlag);

    QString statusTip() const;
    QString status() const;
    void addStatus(const QString& status);

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
    const QCborMap m_request;
    QString m_uid;
    QString m_path;
    QBuffer m_buffer;
    QList<Block> m_recentBlocks;

private:
    bool m_errorFlag;
    QString m_status;
    QString m_statusTip;
    QTime m_timeLeft;
    qreal m_speed;
    int m_totalBytes;
    int m_transferredBytes;
    int m_state;
};

#endif // BUILDINFO_H
