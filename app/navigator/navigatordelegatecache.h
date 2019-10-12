#ifndef NAVIGATORDELEGATECACHE_H
#define NAVIGATORDELEGATECACHE_H

#include <QStack>

class QTreeWidgetItem;
class NavigatorDelegateCache final
{
public:
    NavigatorDelegateCache() = default;
    NavigatorDelegateCache(const NavigatorDelegateCache&) = delete;
    NavigatorDelegateCache& operator=(const NavigatorDelegateCache&) = delete;
    ~NavigatorDelegateCache();

    void push(QTreeWidgetItem* item);
    QTreeWidgetItem* pop();

private:
    QStack<QTreeWidgetItem*> m_items;
};

#endif // NAVIGATORDELEGATECACHE_H
