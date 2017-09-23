#ifndef FILELIST_H
#define FILELIST_H

#include <QTreeView>
#include <QFileSystemModel>

class FileList : public QTreeView
{
        Q_OBJECT
    public:
        explicit FileList(QWidget *parent = 0);
        QFileSystemModel* fileModel();
        QString currentPath() const;

    public slots:
        void goPath(const QString& path);

    protected:
        virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

    signals:
        void fileOpened(const QString& filePath);

    private:
        QFileSystemModel _fileModel;
};

#endif // FILELIST_H
