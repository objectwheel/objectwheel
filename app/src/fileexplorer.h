#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>

class FileExplorerPrivate;

class FileExplorer : public QWidget
{
        // TODO: Drag & Drop files into explorer and copy them to current dir
        Q_OBJECT
    public:
        explicit FileExplorer(QWidget *parent = 0);
        void setRootPath(const QString& rootPath);
        QString rootPath() const;

    signals:
        void fileOpened(const QString& filePath);

    private:
        FileExplorerPrivate* _d;

};

#endif // FILEEXPLORER_H
