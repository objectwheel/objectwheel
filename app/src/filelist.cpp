#include <filelist.h>
#include <QDebug>
#include <QMouseEvent>
#include <QHeaderView>

FileList::FileList(QWidget *parent) : QTreeView(parent)
{
    _fileModel.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    _fileModel.setRootPath(QDir::currentPath());

    setModel(&_fileModel);
    setRootIndex(_fileModel.index(QDir::currentPath()));
    setItemsExpandable(false);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    header()->setSectionsMovable(false);
}

void FileList::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto _index = indexAt(event->pos());
    auto index = _fileModel.index(_index.row(), 0, rootIndex());
    if (!index.isValid())
        return;
    if (_fileModel.isDir(index))
        setRootIndex(index);
    else
        emit fileOpened(_fileModel.filePath(index));
}

QFileSystemModel* FileList::fileModel()
{
    return &_fileModel;
}

QString FileList::currentPath() const
{
    return _fileModel.filePath(rootIndex());
}

void FileList::goPath(const QString& path)
{
    setRootIndex(_fileModel.index(path));
}
