#ifndef CONTROLSDELEGATECACHE_H
#define CONTROLSDELEGATECACHE_H

#include <QStack>

class QTreeWidgetItem;
class ControlsDelegateCache final
{
public:
    ControlsDelegateCache() = default;
    ControlsDelegateCache(const ControlsDelegateCache&) = delete;
    ControlsDelegateCache& operator=(const ControlsDelegateCache&) = delete;
    ~ControlsDelegateCache();

    void push(QTreeWidgetItem* item);
    QTreeWidgetItem* pop();

private:
    QStack<QTreeWidgetItem*> m_items;
};

#endif // CONTROLSDELEGATECACHE_H
