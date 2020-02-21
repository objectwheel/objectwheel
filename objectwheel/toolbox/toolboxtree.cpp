#include <toolboxtree.h>
#include <toolboxitem.h>
#include <toolboxdelegate.h>
#include <toolutils.h>
#include <parserutils.h>

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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegate(new ToolboxDelegate(this));
    connect(this, &QTreeWidget::itemPressed, this, &ToolboxTree::onItemPress);
}

ToolboxItem* ToolboxTree::addTool(const QString& dir)
{
    const QString& module = ParserUtils::module(dir);
    const QString& category = ToolUtils::toolCetegory(module);

    ToolboxItem* topItem = categoryItem(category);
    if (topItem == 0) {
        topItem = new ToolboxItem;
        topItem->setText(0, category);
        addTopLevelItem(topItem);
        topItem->setExpanded(true);
    }

    ToolboxItem* item = new ToolboxItem;
    item->setDir(dir);
    item->setModule(module);
    item->setText(0, ParserUtils::moduleTypeName(module));
    item->setIcon(0, QIcon(ToolUtils::toolIconPath(module)));
    topItem->addChild(item);
    return item;
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
        item->setExpanded(!item->isExpanded());
        return;
    }

    emit itemPressed(static_cast<ToolboxItem*>(item));
}
