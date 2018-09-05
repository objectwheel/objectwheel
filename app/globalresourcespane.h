#ifndef GLOBALRESOURCESPANE_H
#define GLOBALRESOURCESPANE_H

#include <QTreeView>
#include <private/qtreeview_p.h>

class FocuslessLineEdit;
class QFileSystemModel;
class QToolBar;
class QToolButton;
class QComboBox;
class PathIndicator;
class FileSystemProxyModel;
class QLabel;
class QCompleter;
class FileSearchModel;

class GlobalResourcesPanePrivate : public QTreeViewPrivate {};
class GlobalResourcesPane : public QTreeView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GlobalResourcesPane)

    friend class GlobalListDelegate;

public:
    explicit GlobalResourcesPane(QWidget* parent = nullptr);

public slots:
    void sweep();

private slots:
    void onProjectStart();
    void onModeChange();
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
    void onDownloadButtonClick();
    void onFileSelectionChange();
    void onItemDoubleClick(const QModelIndex& index);

private:
    void filterList();
    void goToPath(const QString& path);
    void goToRelativePath(const QString& relativePath);
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void updateGeometries() override;
    QSize sizeHint() const override;

signals:
    void fileOpened(const QString& path);
    void filesDeleted(const QSet<QString>& pathes);

private:
    QLabel* m_dropHereLabel;
    QGraphicsBlurEffect* m_droppingBlurEffect;
    FileSearchModel* m_searchEditCompleterModel;
    QCompleter* m_searchEditCompleter;
    FocuslessLineEdit* m_searchEdit;
    QFileSystemModel* m_fileSystemModel;
    FileSystemProxyModel* m_fileSystemProxyModel;
    QToolBar* m_toolBar;
    PathIndicator* m_pathIndicator;
    QComboBox* m_modeComboBox;
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
    QToolButton* m_downloadFileButton;
    enum Modes { Viewer, Explorer} m_mode;
};

#endif // GLOBALRESOURCESPANE_H