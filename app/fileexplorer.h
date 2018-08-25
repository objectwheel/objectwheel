#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QFrame>

class FileExplorerPrivate;

class FileExplorer : public QFrame
{
    Q_OBJECT

public:
    explicit FileExplorer(QWidget *parent = 0);
    void setRootPath(const QString& rootPath);
    QString rootPath() const;

public slots:
    void sweep();

signals:
    void fileOpened(const QString& filePath);
    void fileDeleted(const QString& filePath);
    void fileRenamed(const QString& filePathFrom, const QString& filePathTo);

protected:
    QSize sizeHint() const override;

private:
    FileExplorerPrivate* _d;

};

#endif // FILEEXPLORER_H
