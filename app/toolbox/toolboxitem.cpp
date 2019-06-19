#include <toolboxitem.h>

ToolboxItem::ToolboxItem(int type) : QTreeWidgetItem(type)
{
}

bool ToolboxItem::isCategory() const
{
    return parent() == 0;
}

QIcon ToolboxItem::icon() const
{
    return QTreeWidgetItem::icon(0);
}

QString ToolboxItem::dir() const
{
    return m_dir;
}

void ToolboxItem::setDir(const QString& dir)
{
    m_dir = dir;
}
