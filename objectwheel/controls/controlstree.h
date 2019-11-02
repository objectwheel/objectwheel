#ifndef CONTROLSTREE_H
#define CONTROLSTREE_H

#include <QTreeWidget>

class ControlsDelegate;
class ControlsTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlsTree)
    Q_DECLARE_PRIVATE(QTreeWidget)

    friend class ControlsDelegate; // For itemFromIndex()

public:
    explicit ControlsTree(QWidget* parent = nullptr);

    ControlsDelegate* delegate() const;
    QList<QTreeWidgetItem*> topLevelItems();
    QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true,
                                             bool includeCollapsed = true);

private:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void paintEvent(QPaintEvent* event) override;

private:
    ControlsDelegate* m_delegate;
};

#endif // CONTROLSTREE_H
