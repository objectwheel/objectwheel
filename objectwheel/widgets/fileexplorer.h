#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QTreeView>
#include <QStack>

class LineEdit;
class QFileSystemModel;
class QToolBar;
class QToolButton;
class QComboBox;
class PathIndicator;
class FileSystemProxyModel;
class QLabel;
class QCompleter;
class FileSearchModel;
class QGraphicsBlurEffect;

class FileExplorer : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTreeView)

    friend class FileExplorerListDelegate;

public:
    enum Mode { Invalid, Viewer, Explorer };
    Q_ENUM(Mode)

public:
    explicit FileExplorer(QWidget* parent = nullptr);

    QString rootPath() const;
    void setRootPath(const QString& rootPath);

    Mode mode() const;
    void setMode(FileExplorer::Mode mode);

public slots:
    void discharge();
    void goToEntry(const QString& entry);
    void goToDir(const QString& dir);
    void goToRelativeDir(const QString& relativeDir);

private slots:
    void onUpButtonClick();
    void onBackButtonClick();
    void onForthButtonClick();
    void onHomeButtonClick();
    void onCopyButtonClick();
    void onPasteButtonClick();
    void onDeleteButtonClick();
    void onNewFileButtonClick();
    void onRenameButtonClick();
    void onNewFolderButtonClick();
    void onFileSelectionChange();
    void onItemDoubleClick(const QModelIndex& index);

protected:
    void setPalette(const QPalette& palette);
    void fillBackground(QPainter* painter, const QStyleOptionViewItem& option,
                        int row, bool verticalLine) const;

private:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void updateGeometries() override;

signals:
    void fileOpened(const QString& relativePath);
    void filesDeleted(const QSet<QString>& pathes);
    void currentDirChanged(const QString& currentDir);

private:
    Mode m_mode;
    QLabel* m_dropHereLabel;
    QGraphicsBlurEffect* m_droppingBlurEffect;
    QFileSystemModel* m_fileSystemModel;
    FileSystemProxyModel* m_fileSystemProxyModel;
    QToolBar* m_toolBar;
    PathIndicator* m_pathIndicator;
    QToolButton* m_upButton;
    QToolButton* m_backButton;
    QToolButton* m_forthButton;
    QToolButton* m_homeButton;
    QToolButton* m_copyButton;
    QToolButton* m_pasteButton;
    QToolButton* m_deleteButton;
    QToolButton* m_renameButton;
    QToolButton* m_newFileButton;
    QToolButton* m_newFolderButton;    
    // FIXME: Rename those
    int lastVScrollerPosOfViewer = 0;
    int lastHScrollerPosOfViewer = 0;
    int lastVScrollerPosOfExplorer = 0;
    int lastHScrollerPosOfExplorer = 0;
    QModelIndexList lastSelectedIndexesOfViewer;
    QModelIndexList lastSelectedIndexesOfExplorer;
    QSet<QPersistentModelIndex> lastExpandedIndexesOfViewer;
    QString lastPathofExplorer;
    QStack<QString> backPathStack;
    QStack<QString> forthPathStack;
};

#endif // FILEEXPLORER_H