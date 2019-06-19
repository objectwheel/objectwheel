#include <toolboxtree.h>
#include <global.h>
#include <toolboxdelegate.h>

#include <QMimeData>
#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QScroller>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>

//!
//! *********************** [ToolboxDelegate] ***********************
//!

const char* TOOL_KEY = "QURBUEFaQVJMSVlJWiBIQUZJWg";

//!
//! ********************** [ToolboxTree] **********************
//!

ToolboxTree::ToolboxTree(QWidget *parent) : QTreeWidget(parent)
{
    setIconSize({20, 20});
    setFocusPolicy(Qt::NoFocus);
    setIndentation(0);
    setRootIsDecorated(false);
    setColumnCount(1);
    header()->hide();
    header()->setSectionResizeMode(QHeaderView::Stretch);
    setTextElideMode(Qt::ElideMiddle);

    setDragEnabled(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragDropMode(QAbstractItemView::DragOnly);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new ToolboxDelegate(this));
}

void ToolboxTree::addUrls(QTreeWidgetItem* item, const QList<QUrl>& urls)
{
    _urls.insert(item, urls);
}

void ToolboxTree::removeUrls(QTreeWidgetItem* item)
{
    _urls.remove(item);
}

void ToolboxTree::clearUrls()
{
    _urls.clear();
}

const QMap<QTreeWidgetItem*, QList<QUrl>>& ToolboxTree::allUrls() const
{
    return _urls;
}

QList<QUrl> ToolboxTree::urls(QTreeWidgetItem* item) const
{
    return _urls.value(item);
}

bool ToolboxTree::contains(const QString& itemName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        for (int j = 0; j < topLevelItem(i)->childCount(); j++)
            if (topLevelItem(i)->child(j)->text(0) == itemName)
                return true;
    return false;
}

bool ToolboxTree::categoryContains(const QString& categoryName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        if (topLevelItem(i)->text(0) == categoryName)
            return true;
    return false;
}

QTreeWidgetItem* ToolboxTree::categoryItem(const QString& categoryName)
{
    for (int i = 0; i < topLevelItemCount(); i++)
        if (topLevelItem(i)->text(0) == categoryName)
            return topLevelItem(i);
    return nullptr;
}

QMimeData* ToolboxTree::mimeData(const QList<QTreeWidgetItem*> items) const
{
    if (itemAt(_pressPoint) &&
            itemAt(_pressPoint)->parent() != nullptr) {
        QMimeData *data = QTreeWidget::mimeData(items);
        data->setUrls(_urls.value(items.first()));
        data->setText(TOOL_KEY);
        return data;
    } else {
        return nullptr;
    }
}

void ToolboxTree::mousePressEvent(QMouseEvent* event)
{
    _pressPoint = event->pos();
    QTreeWidget::mousePressEvent(event);
}
