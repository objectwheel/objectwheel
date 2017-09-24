#ifndef FILELIST_H
#define FILELIST_H

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

class FileFilterProxyModel : public QSortFilterProxyModel
{
    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
        {
            QModelIndex _index = sourceModel()->index(sourceRow, 0, sourceParent);
            QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
            if (fileModel->fileName(_index).startsWith("_"))
                return false;
            else
                return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
        }
};

class FileList : public QTreeView
{
        Q_OBJECT
    public:
        explicit FileList(QWidget *parent = 0);
        QFileSystemModel* fileModel();
        FileFilterProxyModel* filterProxyModel();
        QString currentPath() const;


    public slots:
        void goPath(const QString& path);

    protected:
        virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

    signals:
        void fileOpened(const QString& filePath);

    private:
        QFileSystemModel _fileModel;
        FileFilterProxyModel _filterProxyModel;

};

#endif // FILELIST_H
