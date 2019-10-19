#include <controlsdelegatecache.h>
#include <QTreeWidgetItem>

ControlsDelegateCache::~ControlsDelegateCache()
{
    for (QTreeWidgetItem* item : qAsConst(m_items))
        delete item;
    m_items.clear();
}

void ControlsDelegateCache::push(QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    m_items.push(item);
}

QTreeWidgetItem* ControlsDelegateCache::pop()
{
    if (m_items.isEmpty())
        return nullptr;
    return m_items.pop();
}
