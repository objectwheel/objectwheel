#ifndef FILELIST_H
#define FILELIST_H

#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QUrl>

class FileFilterProxyModel : public QSortFilterProxyModel
{
    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
        {
            QModelIndex _index = sourceModel()->index(sourceRow, 0, sourceParent);
            QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
            auto fileName = fileModel->fileName(_index);
            if (fileName.startsWith("_") || fileName == "icon.png")
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
        ~FileList();
        QFileSystemModel* fileModel();
        FileFilterProxyModel* filterProxyModel();
        QString currentPath() const;

    public slots:
        void goPath(const QString& path);

    private slots:
        void handleDrop(const QList<QUrl>& urls);

    protected:
        virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
        virtual void dragEnterEvent(QDragEnterEvent *event) override;
        virtual void dropEvent(QDropEvent *event) override;
        virtual void dragMoveEvent(QDragMoveEvent *event) override;
        virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
        virtual void resizeEvent(QResizeEvent *event) override;

    signals:
        void fileOpened(const QString& filePath);

    private:
        QFileSystemModel _fileModel;
        FileFilterProxyModel _filterProxyModel;
};

#endif // FILELIST_H
