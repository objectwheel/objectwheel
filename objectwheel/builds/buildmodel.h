#ifndef BUILDMODEL_H
#define BUILDMODEL_H

#include <QAbstractListModel>

class Build;
class QCborMap;

class BuildModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildModel)

public:
    enum BuildRoles {
        ButtonSize = Qt::UserRole + 1,
        NameRole,
        PlatformIconRole,
        VersionRole,
        AbisRole,
        StatusRole,
        SpeedRole,
        TimeLeftRole,
        TotalBytesRole,
        ReceivedBytesRole
    };

public:
    explicit BuildModel(QObject* parent = nullptr);

    void addBuildRequest(const QCborMap& request);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

public slots:
    void clear();

private slots:
    void onServerResponse(const QByteArray& data);

private:
    void scheduleConnection(Build* build);
    void establishConnection(Build* build);
    QIcon platformIcon(const QString& rawPlatformName) const;
    QString packageSuffixFromRequest(const QCborMap& request) const;
    Build* buildFromUid(const QString& uid);

private:
    QList<Build*> m_builds;
};

#endif // BUILDMODEL_H
