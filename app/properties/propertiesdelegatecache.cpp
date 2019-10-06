#include <propertiesdelegatecache.h>
#include <propertiesdelegate.h>
#include <QTreeWidgetItem>

PropertiesDelegateCache::~PropertiesDelegateCache()
{
    for (WidgetStack* stack : qAsConst(m_widgets)) {
        const WidgetStack& _stack = *stack;
        for (QWidget* widget : _stack)
            delete widget;
        delete stack;
    }
    m_widgets.clear();
    for (QTreeWidgetItem* item : qAsConst(m_items))
        delete item;
    m_items.clear();
}

void PropertiesDelegateCache::push(int type, QWidget* widget)
{
    Q_ASSERT(widget);
    Q_ASSERT(type != PropertiesDelegate::Invalid);
    if (!m_widgets.contains(type))
        m_widgets.insert(type, new WidgetStack);
    m_widgets.value(type)->push(widget);
}

void PropertiesDelegateCache::push(QTreeWidgetItem* item)
{
    Q_ASSERT(item);
    m_items.push(item);
}

QWidget* PropertiesDelegateCache::pop(int type)
{
    Q_ASSERT(type != PropertiesDelegate::Invalid);
    if (WidgetStack* stack = m_widgets.value(type, nullptr)) {
        if (!stack->isEmpty())
            return stack->pop();
    }
    return nullptr;
}

QTreeWidgetItem* PropertiesDelegateCache::pop()
{
    if (m_items.isEmpty())
        return nullptr;
    return m_items.pop();
}
