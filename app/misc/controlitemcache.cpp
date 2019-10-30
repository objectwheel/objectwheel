#include <controlitemcache.h>
#include <control.h>
#include <QTreeWidgetItem>

ControlItemCache::~ControlItemCache()
{
    for (QTreeWidgetItem* item : qAsConst(m_items))
        delete item;
    m_items.clear();
}

void ControlItemCache::push(QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    m_items.push(item);
}

QTreeWidgetItem* ControlItemCache::pop()
{
    if (m_items.isEmpty())
        return nullptr;
    return m_items.pop();
}

bool ControlItem::operator<(const QTreeWidgetItem& other) const
{
    const auto& myControl = data(0, ControlRole).value<QPointer<Control>>();
    const auto& otherControl = other.data(0, ControlRole).value<QPointer<Control>>();
    Q_ASSERT(myControl);
    Q_ASSERT(otherControl);
    if (myControl && otherControl)
        return myControl->index() < otherControl->index();
    return false;
}
