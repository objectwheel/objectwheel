#ifndef PROJECTLISTMODEL_H
#define PROJECTLISTMODEL_H

#include <QHash>
#include <QJsonObject>
#include <QAbstractListModel>

class ProjectListModel : public QAbstractListModel
{
        Q_OBJECT
        Q_DISABLE_COPY(ProjectListModel)

    public:
        enum Roles {
            ProjectNameRole = Qt::UserRole + 1,
            LastEditedRole = Qt::UserRole + 2,
            ActiveRole = Qt::UserRole + 3
        };

        struct ProjectProperty {
            QString projectName;
            QString lastEdited;
            bool active;
        };

    public:
        explicit ProjectListModel(QObject* parent = nullptr);

        Q_INVOKABLE QJsonObject get(int row) const;
        Q_INVOKABLE QVariant get(int row, const QString& property) const;
        Q_INVOKABLE bool set(int row, const QString& property, const QVariant& data);

        QHash<int, QByteArray> roleNames() const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int, const QModelIndex& parent = QModelIndex()) const override;

        void clear();
        int roleFromString(const QString& roleName) const;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
        bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    private:
        QHash<int, QByteArray> _roleNames;
        QList<ProjectProperty> _data;
};


#endif // PROJECTLISTMODEL_H