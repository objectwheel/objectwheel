#include <toolboxitem.h>
#include <toolutils.h>

ToolboxItem::ToolboxItem(int type) : QTreeWidgetItem(type)
{
    setFlags(Qt::ItemIsEnabled);
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

QString ToolboxItem::module() const
{
    return m_module;
}

void ToolboxItem::setModule(const QString& module)
{
    m_module = module;
}

bool ToolboxItem::operator<(const QTreeWidgetItem& other) const
{
    if (isCategory())
        return ToolUtils::toolCetegoryIndex(text(0)) < ToolUtils::toolCetegoryIndex(other.text(0));
    return QTreeWidgetItem::operator<(other);
}
