#include <navigatordelegate.h>
#include <navigatortree.h>
#include <navigatordelegatecache.h>
#include <utilityfunctions.h>
#include <control.h>

#include <QPointer>
#include <QPainter>

class NavigatorItem final : public QTreeWidgetItem {
    bool operator<(const QTreeWidgetItem& other) const {
        const auto& myControl = data(0, NavigatorDelegate::ControlRole).value<QPointer<Control>>();
        const auto& otherControl = other.data(0, NavigatorDelegate::ControlRole).value<QPointer<Control>>();
        Q_ASSERT(myControl);
        Q_ASSERT(otherControl);
        if (myControl && otherControl)
            return myControl->index() < otherControl->index();
        return false;
    }
};

NavigatorDelegate::NavigatorDelegate(NavigatorTree* navigatorTree) : QStyledItemDelegate(navigatorTree)
  , m_navigatorTree(navigatorTree)
  , m_cache(new NavigatorDelegateCache)
{
}

NavigatorDelegate::~NavigatorDelegate()
{
    delete m_cache;
}

void NavigatorDelegate::reserve()
{
    for (int i = 200; i--;) {
        auto item = new QTreeWidgetItem;
        item->setHidden(true);
        m_cache->push(item);
    }
}

void NavigatorDelegate::destroyItem(QTreeWidgetItem* item) const
{
    if (QTreeWidgetItem* parent = item->parent())
        parent->takeChild(parent->indexOfChild(item));
    else if (QTreeWidget* tree = item->treeWidget())
        tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));
    item->setHidden(true);
    item->setData(0, ControlRole, QVariant());
    item->setData(1, ControlRole, QVariant());
    m_cache->push(item);
}

QTreeWidgetItem* NavigatorDelegate::createItem(Control* control) const
{
    QTreeWidgetItem* item = m_cache->pop();
    if (item == 0)
        item = new QTreeWidgetItem;
    item->setData(0, ControlRole, QVariant::fromValue(QPointer<Control>(control)));
    item->setData(1, ControlRole, QVariant::fromValue(QPointer<Control>(control)));
    return item;
}

int NavigatorDelegate::calculateVisibleRow(const QTreeWidgetItem* item) const
{
    int count = 0;
    const QList<QTreeWidgetItem*>& topLevelItems = m_navigatorTree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& subs = m_navigatorTree->allSubChildItems(topLevelItem, true, false);
        for (QTreeWidgetItem* childItem : subs) {
            if (childItem == item)
                return count;
            ++count;
        }
    }
    Q_ASSERT(false);
    return -1;
}

void NavigatorDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
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

void NavigatorDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    painter->save();

    const bool isSelected = option.state & QStyle::State_Selected;
    const QAbstractItemModel* model = index.model();
    const QRectF r = option.rect;
    const auto& control = model->data(index, ControlRole).value<QPointer<Control>>();
    Q_ASSERT(control);

    paintBackground(painter, option, calculateVisibleRow(m_navigatorTree->itemFromIndex(index)),
                    index.column() == 0);

    // Draw icon
    QRectF iconRect(0, 0, -5, 0);
    if (index.column() == 0) {
        Q_ASSERT(UtilityFunctions::window(m_navigatorTree));
        const QPixmap& iconPixmap = control->icon().pixmap(UtilityFunctions::window(m_navigatorTree),
                                                           option.decorationSize,
                                                           isSelected ? QIcon::Selected : QIcon::Normal);
        iconRect = QRectF({}, iconPixmap.size() / m_navigatorTree->devicePixelRatioF());
        iconRect.moveCenter(r.center());
        iconRect.moveLeft(r.left() + 5);
        painter->drawPixmap(iconRect, iconPixmap, iconPixmap.rect());
    }

    // Draw text
    if (control && control->hasErrors() && isSelected)
        painter->setPen(option.palette.linkVisited().color().lighter(140));
    else if (control && control->hasErrors() && !isSelected)
        painter->setPen(option.palette.linkVisited().color());
    else if (isSelected)
        painter->setPen(option.palette.highlightedText().color());
    else
        painter->setPen(option.palette.text().color());

    if (control) {
        const QRectF& textRect = r.adjusted(iconRect.width() + 10, 0, 0, 0);
        const QString& text = index.column() == 0
                ? control->id()
                : control->gui() && !control->hasErrors() ? tr("Yes") : tr("No");
        painter->drawText(textRect,
                          option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    }

    painter->restore();
}
