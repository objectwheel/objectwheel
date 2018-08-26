#ifndef GLOBALRESOURCESPANE_H
#define GLOBALRESOURCESPANE_H

#include <QTreeView>
#include <private/qtreeview_p.h>

class FocuslessLineEdit;
class QFileSystemModel;
class QToolBar;
class QToolButton;
class QComboBox;

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
    void onHomeButtonClick();
    void onCopyButtonClick();
    void onDeleteButtonClick();
    void onNewFileButtonClick();
    void onRenameButtonClick();
    void onNewFolderButtonClick();
    void onDownloadButtonClick();
    void onFileSelectionChange();

private:
    void filterList(const QString& filter);
    void paintEvent(QPaintEvent* e) override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void updateGeometries() override;
    QSize sizeHint() const override;

private:
    FocuslessLineEdit* m_searchEdit;
    QFileSystemModel* m_fileSystemModel;
    QToolBar* m_toolbar;
    QComboBox* m_modeComboBox;
    QToolButton* m_upButton;
    QToolButton* m_homeButton;
    QToolButton* m_copyButton;
    QToolButton* m_deleteButton;
    QToolButton* m_renameButton;
    QToolButton* m_newFileButton;
    QToolButton* m_newFolderButton;
    QToolButton* m_downloadFileButton;
};

#endif // GLOBALRESOURCESPANE_H