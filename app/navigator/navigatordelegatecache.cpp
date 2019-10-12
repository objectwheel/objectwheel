#include <navigatordelegatecache.h>
#include <QTreeWidgetItem>

NavigatorDelegateCache::~NavigatorDelegateCache()
{
    for (QTreeWidgetItem* item : qAsConst(m_items))
        delete item;
    m_items.clear();
}

void NavigatorDelegateCache::push(QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    m_items.push(item);
}

QTreeWidgetItem* NavigatorDelegateCache::pop()
{
    if (m_items.isEmpty())
        return nullptr;
    return m_items.pop();
}
