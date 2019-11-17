#ifndef FILEEXPLORER_P_H
#define FILEEXPLORER_P_H

#include <QTreeView>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QFileIconProvider>

class QFileSystemWatcher;
class FileExplorer;

class FileExplorerListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FileExplorerListDelegate(FileExplorer* parent);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    FileExplorer* m_fileExplorer;
};

class PathIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit PathIndicator(QWidget* parent = nullptr);
    void setPath(const QString& path);

signals:
    void pathUpdated(const QString& newPath);

private:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
    QString m_path;
    QLineEdit* m_pathEdit;
};

class FileSystemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit FileSystemProxyModel(QObject* parent = nullptr);

private:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

class FileSearchModel : public QStringListModel
{
    Q_OBJECT

public:
    explicit FileSearchModel(QObject* parent = nullptr);
    void setRootPath(const QString& rootPath);

private slots:
    void updateModel();

private:
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QString m_rootPath;
    QFileIconProvider m_fileIconProvider;
    QFileSystemWatcher* m_fileSystemWatcher;
};

#endif // FILEEXPLORER_P_H