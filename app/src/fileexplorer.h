#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>

class FileExplorerPrivate;

class FileExplorer : public QWidget
{
        Q_OBJECT
    public:
        explicit FileExplorer(QWidget *parent = 0);
        void setRootPath(const QString& rootPath);
        QString rootPath() const;

    signals:
        void fileOpened(const QString& filePath);
        void fileDeleted(const QString& filePath);
        void fileRenamed(const QString& filePathFrom, const QString& filePathTo);

    private:
        FileExplorerPrivate* _d;

};

#endif // FILEEXPLORER_H
