#ifndef BUILDMODEL_H
#define BUILDMODEL_H

#include <QSet>
#include <QHash>
#include <QBasicTimer>
#include <QAbstractListModel>

class BuildInfo;
class QCborMap;
class PayloadRelay;

class BuildModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildModel)

public:
    enum BuildRoles {
        Identifier = Qt::UserRole + 1,
        ButtonSize,
        ErrorRole,
        StateRole,
        StatusRole,
        NameRole,
        PathRole,
        PlatformIconRole,
        VersionRole,
        AbisRole,
        SpeedRole,
        TimeLeftRole,
        TotalBytesRole,
        TransferredBytesRole
    };

    enum State {
        Uploading,
        Downloading,
        Finished
    };

public:
    explicit BuildModel(QObject* parent = nullptr);
    ~BuildModel() override;

    void addBuildRequest(const QCborMap& request);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    QModelIndex indexFromIdentifier(const QString& identifier) const;

public slots:
    void clear();

private slots:
    void start(BuildInfo* buildInfo);
    void onServerResponse(const QByteArray& data);
    void emitDelayedDataChanged(const QModelIndex& index, const QVector<int>& roles);
    void onPayloadBytesUploaded(const QString& uid, int bytes);
    void onPayloadBytesDownloaded(const QString& payloadUid, const QByteArray& chunk, int totalBytes);
    void onPayloadUploadFinished(const QString& payloadUid);
    void onPayloadDownloadFinished(const QString& payloadUid, const QByteArray& data);
    void onPayloadUploadTimedout(const QString& payloadUid);
    void onPayloadDownloadTimedout(const QString& payloadUid);

private:
    void timerEvent(QTimerEvent* event) override;

signals:
    void uploadFinished(const QModelIndex& index);
    void downloadFinished(const QModelIndex& index);

private:
    QIcon platformIcon(const QString& rawPlatformName) const;
    BuildInfo* buildInfoFromUid(const QString& uid) const;
    BuildInfo* buildInfoFromPayloadUid(const QString& payloadUid) const;
    QModelIndex indexFromBuildInfo(const BuildInfo* buildInfo) const;
    void calculateTransferRate(BuildInfo* buildInfo, int chunkSize, QSet<int>& changedRoles) const;

private:
    PayloadRelay* m_payloadRelay;
    QList<BuildInfo*> m_buildInfos;
    QBasicTimer m_changeSignalTimer;
    QHash<int, QSet<int>> m_changedRows;
};

#endif // BUILDMODEL_H
