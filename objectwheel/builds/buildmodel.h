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
        PaddingRole = Qt::UserRole + 1,
        PlatformRole,
        NameRole,
        PlatformIconRole,
        VersionRole,
        AbisRole,
        StatusRole,
        SpeedRole,
        TimeLeftRole,
        TotalDataSizeRole,
        ReceivedDataSizeRole
    };

public:
    explicit BuildModel(QObject* parent = nullptr);

    void addBuildRequest(const QCborMap& request);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private slots:
    void onServerResponse(const QByteArray& data);

private:
    void scheduleConnection(Build* build);
    void establishConnection(Build* build);

    QIcon platformIcon(const QString& rawPlatformName) const;
    QString toPrettyPlatformName(const QString& rawPlatformName) const;
    QString packageSuffixFromRequest(const QCborMap& request) const;

private:
    QList<Build*> m_builds;
};

#endif // BUILDMODEL_H
