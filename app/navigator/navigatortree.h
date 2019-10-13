#ifndef NAVIGATORTREE_H
#define NAVIGATORTREE_H

#include <QTreeWidget>

class NavigatorDelegate;
class NavigatorTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(NavigatorTree)
    Q_DECLARE_PRIVATE(QTreeWidget)

    friend class NavigatorDelegate; // For itemFromIndex()

public:
    explicit NavigatorTree(QWidget* parent = nullptr);

    NavigatorDelegate* delegate() const;
    QList<QTreeWidgetItem*> topLevelItems();
    QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true,
                                             bool includeCollapsed = true);

private:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void paintEvent(QPaintEvent* event) override;

private:
    NavigatorDelegate* m_delegate;
};

#endif // NAVIGATORTREE_H
