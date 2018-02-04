#include <projectlistmodel.h>

ProjectListModel::ProjectListModel(QObject* parent) : QAbstractListModel(parent)
{
    _roleNames[ProjectNameRole] = "projectName";
    _roleNames[LastEditedRole] = "lastEdited";
    _roleNames[ActiveRole] = "active";
}

QHash<int, QByteArray> ProjectListModel::roleNames() const
{
    return _roleNames;
}

int ProjectListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return _data.size();
}

QVariant ProjectListModel::data(const QModelIndex& index, int role) const
{
    QVariant rv;

    if (index.row() >= _data.size())
        return rv;

    switch (role)
    {
        case ProjectNameRole:
            rv = _data.at(index.row()).projectName;
            break;
        case LastEditedRole:
            rv = _data.at(index.row()).lastEdited;
            break;
        case ActiveRole:
            rv = _data.at(index.row()).active;
            break;
        default:
            break;
    }
    return rv;
}

QModelIndex ProjectListModel::index(int row, int, const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, 0);
}

bool ProjectListModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        _data.insert(row, ProjectProperty());

    endInsertRows();

    return true;
}

bool ProjectListModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        _data.removeAt(row);

    endRemoveRows();

    return true;
}

bool ProjectListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.row() >= 0 && index.row() < _data.size()
        && _roleNames.keys().contains(role)) {
        switch (role)
        {
            case ProjectNameRole:
                _data[index.row()].projectName = value.toString();
                break;
            case LastEditedRole:
                _data[index.row()].lastEdited = value.toString();
                break;
            case ActiveRole:
                _data[index.row()].active = value.toBool();
                break;
            default:
                break;
        }
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

void ProjectListModel::clear()
{
    emit beginResetModel();
    _data.clear();
    emit endResetModel();
}

Qt::ItemFlags ProjectListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractListModel::flags(index);

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

QJsonObject ProjectListModel::get(int row) const // For QML side usage
{
    QJsonObject jobj;
    if (row >= 0 && row < _data.size()) {
        jobj[_roleNames[ProjectNameRole]] = _data.at(row).projectName;
        jobj[_roleNames[LastEditedRole]] = _data.at(row).lastEdited;
        jobj[_roleNames[ActiveRole]] = _data.at(row).active;
        return jobj;
    } else {
        return jobj;
    }
}

QVariant ProjectListModel::get(int row, const QString& property) const
{
    int role = roleFromString(property);
    if (role >= 0 && row >= 0)
        return data(index(row, 0), role);
    else
        return QVariant();
}

bool ProjectListModel::set(int row, const QString& property, const QVariant& data)
{
    int role = roleFromString(property);
    if (role >= 0 && row >= 0) return setData(index(row, 0), data, role);
    else return false;
}

int ProjectListModel::roleFromString(const QString& roleName) const
{
    return _roleNames.key(roleName.toLatin1(), -1);
}
