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
        ButtonSize = Qt::UserRole + 1,
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

public slots:
    void clear();

private slots:
    void start();
    void onServerResponse(const QByteArray& data);
    void onServerBytesWritten(qint64 bytes);
    void emitDelayedDataChanged(const QModelIndex& index, const QVector<int>& roles);
    void onPayloadBytesDownload(const QString& payloadUid, const QByteArray& chunk, int totalBytes);
    void onPayloadDownloadFinish(const QString& payloadUid, const QByteArray& data);

private:
    void timerEvent(QTimerEvent* event) override;

signals:
    void uploadFinished(const QModelIndex& index);
    void downloadFinished(const QModelIndex& index);

private:
    QIcon platformIcon(const QString& rawPlatformName) const;
    BuildInfo* uploadingBuildInfo() const;
    BuildInfo* buildInfoFromUid(const QString& uid);
    BuildInfo* buildInfoFromPayloadUid(const QString& payloadUid);
    QModelIndex indexFromBuildInfo(const BuildInfo* buildInfo) const;

private:
    PayloadRelay* m_payloadRelay;
    QList<BuildInfo*> m_buildInfos;
    QBasicTimer m_changeSignalTimer;
    QHash<int, QSet<int>> m_changedRows;
};

#endif // BUILDMODEL_H
