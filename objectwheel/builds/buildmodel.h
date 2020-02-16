#ifndef BUILDMODEL_H
#define BUILDMODEL_H

#include <QAbstractListModel>

struct Build
{
    QString name;
    QString version;
    QString abis;
    QString status;
};

class BuildModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildModel)

public:
    enum BuildRoles {
        PlatformRole = Qt::UserRole + 1,
        NameRole,
        IconRole,
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

    void addBuild(const Build& build);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private:
    QList<Build> m_builds;
};

#endif // BUILDMODEL_H
