#ifndef PROPERTIESTREE_H
#define PROPERTIESTREE_H

#include <QTreeWidget>

class PropertiesDelegate;
class PropertiesCache;

class PropertiesTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesTree)

    friend class PropertiesDelegate; // For itemFromIndex()

public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        PropertyNameRole,
        InitialValueRole,
        ModificationRole
    };

public:
    explicit PropertiesTree(QWidget* parent = nullptr);

    PropertiesDelegate* delegate() const;
    PropertiesCache* cache() const;

    QList<QTreeWidgetItem*> topLevelItems() const;
    QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem,
                                             bool includeParent = true,
                                             bool includeCollapsed = true,
                                             bool includeHidden = false) const;

private:
    void paintEvent(QPaintEvent* event);
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;

private:
    PropertiesDelegate* m_delegate;
    PropertiesCache* m_cache;
};

#endif // PROPERTIESTREE_H
