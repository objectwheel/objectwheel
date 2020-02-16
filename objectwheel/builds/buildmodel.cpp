#include <buildmodel.h>
#include <servermanager.h>

BuildModel::BuildModel(QObject* parent) : QAbstractListModel(parent)
{
}

void BuildModel::addBuild(const Build& build)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_builds.append(build);
    endInsertRows();
}

int BuildModel::rowCount(const QModelIndex&) const
{
    return m_builds.count();
}

QVariant BuildModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_builds.count())
        return QVariant();

    const Build& build = m_builds[index.row()];
    // if (role == TypeRole)
    //     return build.type();
    // else if (role == SizeRole)
    //     return build.size();
    return QVariant();
}
