#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>

class FileExplorerPrivate;

class FileExplorer : public QWidget
{
        Q_OBJECT
    public:
        explicit FileExplorer(QWidget *parent = 0);

    private:
        FileExplorerPrivate* _d;

};

#endif // FILEEXPLORER_H
