#ifndef GLOBALRESOURCESPANE_H
#define GLOBALRESOURCESPANE_H

#include <QTreeView>
#include <private/qtreeview_p.h>

class FocuslessLineEdit;
class QFileSystemModel;

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

private:
    void filterList(const QString& filter);
    void paintEvent(QPaintEvent* e) override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void updateGeometries() override;
    QSize sizeHint() const override;

private:
    FocuslessLineEdit* m_searchEdit;
    QFileSystemModel* m_fileSystemModel;
};

#endif // GLOBALRESOURCESPANE_H