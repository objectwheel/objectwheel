#include <filelist.h>
#include <QDebug>
#include <QMouseEvent>
#include <QHeaderView>

FileList::FileList(QWidget *parent) : QTreeView(parent)
{
    _fileModel.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    setItemsExpandable(false);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    header()->setSectionsMovable(false);

    _filterProxyModel.setDynamicSortFilter(true);
    _filterProxyModel.setFilterKeyColumn(0);
    _filterProxyModel.setSourceModel(&_fileModel);
}

void FileList::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto _index = _filterProxyModel.mapToSource(indexAt(event->pos()));
    auto index = _fileModel.index(_index.row(), 0, _filterProxyModel.
                                  mapToSource(rootIndex()));
    if (!index.isValid())
        return;
    if (_fileModel.isDir(index))
        setRootIndex(_filterProxyModel.mapFromSource(index));
    else
        emit fileOpened(_fileModel.filePath(index));
}

FileFilterProxyModel* FileList::filterProxyModel()
{
    return &_filterProxyModel;
}

QFileSystemModel* FileList::fileModel()
{
    return &_fileModel;
}

QString FileList::currentPath() const
{
    return _fileModel.filePath(_filterProxyModel.mapToSource(rootIndex()));
}

void FileList::goPath(const QString& path)
{
    setRootIndex(_filterProxyModel.mapFromSource(_fileModel.index(path)));
}
