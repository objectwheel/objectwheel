#include <formsdelegate.h>
#include <formstree.h>
#include <paintutils.h>
#include <control.h>
#include <controlitemcache.h>

#include <QPointer>
#include <QPainter>

FormsDelegate::FormsDelegate(FormsTree* formsTree) : QStyledItemDelegate(formsTree)
  , m_formsTree(formsTree)
  , m_cache(new ControlItemCache)
{
}

FormsDelegate::~FormsDelegate()
{
    delete m_cache;
}

void FormsDelegate::reserve()
{
    for (int i = 20; i--;) {
        auto item = new ControlItem;
        item->setHidden(true);
        m_cache->push(item);
    }
}

void FormsDelegate::destroyItem(QTreeWidgetItem* item) const
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

QTreeWidgetItem* FormsDelegate::createItem(Control* control) const
{
    QTreeWidgetItem* item = m_cache->pop();
    if (item == 0)
        item = new ControlItem;
    item->setData(0, ControlItem::ControlRole, QVariant::fromValue(QPointer<Control>(control)));
    item->setData(1, ControlItem::ControlRole, QVariant::fromValue(QPointer<Control>(control)));
    return item;
}

void FormsDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                    int rowNumber) const
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

    painter->restore();
}

void FormsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    painter->save();

    const bool isSelected = option.state & QStyle::State_Selected;
    const QAbstractItemModel* model = index.model();
    const QRectF r = option.rect;
    const auto& control = model->data(index, ControlItem::ControlRole).value<QPointer<Control>>();
    Q_ASSERT(control);

    paintBackground(painter, option, index.row());

    // Draw icon
    QRectF iconRect(0, 0, -5, 0);
    QPixmap pixmap(PaintUtils::pixmap(control->icon(), option.decorationSize, m_formsTree,
                                      isSelected ? QIcon::Selected : QIcon::Normal));
    iconRect = QRectF({}, pixmap.size() / pixmap.devicePixelRatioF());
    iconRect.moveCenter(r.center());
    iconRect.moveLeft(r.left() + 5);
    painter->drawPixmap(iconRect, pixmap, pixmap.rect());

    if (isSelected)
        painter->setPen(option.palette.highlightedText().color());
    else
        painter->setPen(option.palette.text().color());

    // Draw text
    if (control) {
        const QRectF& textRect = r.adjusted(iconRect.width() + 10, 0, 0, 0);
        const QString& text = control->id();
        painter->drawText(textRect, option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    }

    painter->restore();
}
