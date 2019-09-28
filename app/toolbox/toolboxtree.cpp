#include <toolboxtree.h>
#include <toolboxitem.h>
#include <toolboxdelegate.h>

#include <QApplication>
#include <QHeaderView>

ToolboxTree::ToolboxTree(QWidget* parent) : QTreeWidget(parent)
{
    header()->hide();
    header()->setSectionResizeMode(QHeaderView::Stretch);

    setIndentation(0);
    setColumnCount(1);
    setRootIsDecorated(false);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollMode(ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegate(new ToolboxDelegate(this));

    connect(this, &QTreeWidget::itemPressed, this, &ToolboxTree::onItemPress);
}

void ToolboxTree::addTool(const QString& name, const QString& category, const QString& dir,
                          const QIcon& icon)
{
    ToolboxItem* topItem = categoryItem(category);
    if (topItem == 0) {
        topItem = new ToolboxItem;
        topItem->setText(0, category);
        addTopLevelItem(topItem);
        topItem->setExpanded(true);
    }
    ToolboxItem* item = new ToolboxItem;
    item->setDir(dir);
    item->setText(0, name);
    item->setIcon(0, icon);
    topItem->addChild(item);
}

ToolboxItem* ToolboxTree::categoryItem(const QString& category)
{
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = topLevelItem(i);
        if (item->text(0) == category)
            return static_cast<ToolboxItem*>(item);
    }
    return nullptr;
}

void ToolboxTree::onItemPress(QTreeWidgetItem* item, int column)
{
    if (item == 0)
        return;

    if (column > 0)
        return;

    if (QApplication::mouseButtons() != Qt::LeftButton)
        return;

    if (item->parent() == 0) {
        setItemExpanded(item, !isItemExpanded(item));
        return;
    }

    emit itemPressed(static_cast<ToolboxItem*>(item));
}
