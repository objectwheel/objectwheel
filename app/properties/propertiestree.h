#ifndef PROPERTIESTREE_H
#define PROPERTIESTREE_H

#include <QTreeWidget>

class PropertiesDelegate;

class PropertiesTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesTree)

    friend class PropertiesDelegate; // For itemFromIndex()

public:
    explicit PropertiesTree(QWidget* parent = nullptr);

    PropertiesDelegate* propertiesDelegate() const;

    QList<QTreeWidgetItem*> topLevelItems() const;
    QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true,
                                             bool includeCollapsed = true, bool includeHidden = false) const;

private:
    void paintEvent(QPaintEvent* event);
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;
};

#endif // PROPERTIESTREE_H
