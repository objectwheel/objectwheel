#ifndef PROPERTIESTREE_H
#define PROPERTIESTREE_H

#include <QTreeWidget>

class DesignerScene;
class PropertiesDelegate;

class PropertiesTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesTree)
    Q_DECLARE_PRIVATE(QTreeWidget)

    friend class PropertiesDelegate; // For itemFromIndex()

public:
    explicit PropertiesTree(QWidget* parent = nullptr);

    void setDesignerScene(DesignerScene* designerScene);

    PropertiesDelegate* delegate() const;
    QList<QTreeWidgetItem*> topLevelItems() const;
    QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem,
                                             bool includeParent = true,
                                             bool includeCollapsed = true,
                                             bool includeHidden = false) const;
private:
    void paintEvent(QPaintEvent* event);
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;

private:
    DesignerScene* m_designerScene;
    PropertiesDelegate* m_delegate;
};

#endif // PROPERTIESTREE_H
