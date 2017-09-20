#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QWidget>

class FileExplorer : public QWidget
{
        Q_OBJECT
    public:
        explicit FileExplorer(QWidget *parent = 0);

    protected:
        virtual QSize sizeHint() const override;
};

#endif // FILEEXPLORER_H
