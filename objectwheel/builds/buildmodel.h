#ifndef BUILDMODEL_H
#define BUILDMODEL_H

#include <QAbstractListModel>

class BuildInfo;
class QCborMap;

class BuildModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildModel)

public:
    enum BuildRoles {
        ButtonSize = Qt::UserRole + 1,
        StateRole,
        ErrorRole,
        DetailsRole,
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

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

public slots:
    void clear();

private slots:
    void start();
    void onServerResponse(const QByteArray& data);
    void onServerBytesWritten(qint64 bytes);

signals:
    void uploadFinished(const QModelIndex& index);
    void downloadFinished(const QModelIndex& index);

private:
    QIcon platformIcon(const QString& rawPlatformName) const;
    BuildInfo* uploadingBuildInfo() const;
    BuildInfo* buildInfoFromUid(const QString& uid);
    QModelIndex indexFromBuildInfo(const BuildInfo* buildInfo) const;

private:
    QList<BuildInfo*> m_buildInfos;
};

#endif // BUILDMODEL_H
