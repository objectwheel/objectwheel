#ifndef PROPERTIESLISTCACHE_H
#define PROPERTIESLISTCACHE_H

#include <QHash>
#include <QStack>

class QWidget;
class QTreeWidgetItem;

class PropertiesDelegateCache final
{
    using WidgetStack = QStack<QWidget*>;
    using WidgetHash = QHash<int, WidgetStack*>;
    using ItemStack = QStack<QTreeWidgetItem*>;

public:
    PropertiesDelegateCache() = default;
    PropertiesDelegateCache(const PropertiesDelegateCache&) = delete;
    PropertiesDelegateCache& operator=(const PropertiesDelegateCache&) = delete;
    ~PropertiesDelegateCache();

    void push(int type, QWidget* widget);
    void push(QTreeWidgetItem* item);

    QWidget* pop(int type);
    QTreeWidgetItem* pop();

private:
    WidgetHash m_widgets;
    ItemStack m_items;
};

#endif // PROPERTIESLISTCACHE_H
