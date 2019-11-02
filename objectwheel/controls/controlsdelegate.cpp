#include <controlsdelegate.h>
#include <controlstree.h>
#include <controlitemcache.h>
#include <paintutils.h>
#include <control.h>

#include <QPointer>
#include <QPainter>

ControlsDelegate::ControlsDelegate(ControlsTree* controlsTree) : QStyledItemDelegate(controlsTree)
  , m_controlsTree(controlsTree)
  , m_cache(new ControlItemCache)
{
}

ControlsDelegate::~ControlsDelegate()
{
    delete m_cache;
}

void ControlsDelegate::reserve()
{
    for (int i = 200; i--;) {
        auto item = new ControlItem;
        item->setHidden(true);
        m_cache->push(item);
    }
}

void ControlsDelegate::destroyItem(QTreeWidgetItem* item) const
{
    if (QTreeWidgetItem* parent = item->parent())
        parent->takeChild(parent->indexOfChild(item));
    else if (QTreeWidget* tree = item->treeWidget())
        tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));
    item->setHidden(true);
    item->setData(0, ControlItem::ControlRole, QVariant());
    item->setData(1, ControlItem::ControlRole, QVariant());
    m_cache->push(item);
}

QTreeWidgetItem* ControlsDelegate::createItem(Control* control) const
{
    QTreeWidgetItem* item = m_cache->pop();
    if (item == 0)
        item = new ControlItem;
    item->setData(0, ControlItem::ControlRole, QVariant::fromValue(QPointer<Control>(control)));
    item->setData(1, ControlItem::ControlRole, QVariant::fromValue(QPointer<Control>(control)));
    return item;
}

int ControlsDelegate::calculateVisibleRow(const QTreeWidgetItem* item) const
{
    int count = 0;
    const QList<QTreeWidgetItem*>& topLevelItems = m_controlsTree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& subs = m_controlsTree->allSubChildItems(topLevelItem, true, false);
        for (QTreeWidgetItem* childItem : subs) {
            if (childItem == item)
                return count;
            ++count;
        }
    }
    Q_ASSERT(false);
    return -1;
}

void ControlsDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                        int rowNumber, bool hasVerticalLine) const
{
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    const QPalette& pal = option.palette;
    const QRectF& rect = option.rect;

    painter->setClipRect(rect);

    // Fill background
    if (isSelected) {
        painter->fillRect(rect, pal.highlight());
    } else {
        if (rowNumber % 2)
            painter->fillRect(rect, pal.alternateBase());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    QColor lineColor(pal.dark().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                      rect.topRight() - QPointF(0.5, 0.0));
    painter->drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                      rect.bottomRight() - QPointF(0.5, 0.0));

    // Draw vertical line
    if (hasVerticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

void ControlsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    painter->save();

    const bool isSelected = option.state & QStyle::State_Selected;
    const QAbstractItemModel* model = index.model();
    const QRectF r = option.rect;
    const auto& control = model->data(index, ControlItem::ControlRole).value<QPointer<Control>>();
    Q_ASSERT(control);

    paintBackground(painter, option, calculateVisibleRow(m_controlsTree->itemFromIndex(index)),
                    index.column() == 0);

    // Draw icon
    QRectF iconRect(0, 0, -5, 0);
    if (index.column() == 0) {
        QPixmap pixmap(PaintUtils::pixmap(control->icon(), option.decorationSize, m_controlsTree,
                                          isSelected ? QIcon::Selected : QIcon::Normal));
        iconRect = QRectF({}, pixmap.size() / pixmap.devicePixelRatioF());
        iconRect.moveCenter(r.center());
        iconRect.moveLeft(r.left() + 5);
        painter->drawPixmap(iconRect, pixmap, pixmap.rect());
    }

    // Draw text
    if (control && control->hasErrors() && !isSelected)
        painter->setPen(option.palette.link().color());
    else if (isSelected)
        painter->setPen(option.palette.highlightedText().color());
    else
        painter->setPen(option.palette.text().color());

    if (control) {
        const QRectF& textRect = r.adjusted(iconRect.width() + 10, 0, 0, 0);
        QString text = index.column() == 0
                ? control->id()
                : control->gui() && !control->hasErrors() ? tr("Yes") : tr("No");
        if (index.column() == 0 && control->hasErrors())
            text.append('!');
        painter->drawText(textRect, option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    }

    painter->restore();
}
