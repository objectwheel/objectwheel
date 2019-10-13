#include <navigatortree.h>
#include <navigatordelegate.h>
#include <private/qtreewidget_p.h>

#include <QHeaderView>
#include <QPainter>

NavigatorTree::NavigatorTree(QWidget* parent) : QTreeWidget(parent)
  , m_delegate(new NavigatorDelegate(this))
{
    m_delegate->reserve();

    header()->setFixedHeight(20);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->resizeSection(0, 220); // Don't resize the last (stretched) column

    headerItem()->setText(0, tr("Controls"));
    headerItem()->setText(1, tr("Ui"));

    setColumnCount(2);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(m_delegate);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPalette p(palette());
    p.setColor(QPalette::Light, "#62A558");
    p.setColor(QPalette::Dark, "#599750");
    p.setColor(QPalette::AlternateBase, "#e8f7e6");
    setPalette(p);
    setStyleSheet(
                QString {
                    "QTreeView {"
                    "    border: 1px solid %1;"
                    "} QHeaderView::section {"
                    "    color: %4;"
                    "    padding-left: 5px;"
                    "    padding-top: 3px;"
                    "    padding-bottom: 3px;"
                    "    border-style: solid;"
                    "    border-left-width: 0px;"
                    "    border-top-width: 0px;"
                    "    border-bottom-color: %1;"
                    "    border-bottom-width: 1px;"
                    "    border-right-color: %1; "
                    "    border-right-width: 1px;"
                    "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                    "                                stop:0 %2, stop:1 %3);"
                    "}"
                    "QHeaderView::section:last{"
                    "    border-left-width: 0px;"
                    "    border-right-width: 0px;"
                    "}"
                }
                .arg(palette().dark().color().darker(120).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name()));
}

NavigatorDelegate* NavigatorTree::delegate() const
{
    return m_delegate;
}

QList<QTreeWidgetItem*> NavigatorTree::topLevelItems()
{
    QList<QTreeWidgetItem*> items;

    for (int i = 0; i < topLevelItemCount(); ++i)
        items.append(topLevelItem(i));

    return items;
}

QList<QTreeWidgetItem*> NavigatorTree::allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent,
                                                        bool includeCollapsed)
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if (!includeCollapsed && !parentItem->isExpanded()) {
        if (includeParent)
            items.append(parentItem);

        return items;
    }

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false, includeCollapsed));
    }

    return items;
}

void NavigatorTree::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isSelected = itemFromIndex(index)->isSelected();

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;
    if (isSelected)
        option.state |= QStyle::State_Selected;
    else if (option.state & QStyle::State_Selected)
        option.state &= ~QStyle::State_Selected;

    m_delegate->paintBackground(painter, option,
                                m_delegate->calculateVisibleRow(itemFromIndex(index)),
                                false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.2);
        pen.setColor(isSelected ? palette().highlightedText().color() : palette().text().color());
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }

    painter->restore();
}

void NavigatorTree::paintEvent(QPaintEvent* event)
{
    Q_D(const QTreeWidget);

    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());

    QColor lineColor(palette().dark().color());
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    qreal height = d->defaultItemHeight;
    qreal rowCount = viewport()->height() / height;
    for (int i = 0; i < rowCount; ++i) {
        QRectF rect(0, i * height, viewport()->width(), height);
        painter.setClipRect(rect);

        // Fill background
        if (i % 2)
            painter.fillRect(rect, palette().alternateBase());

        // Draw vertical line
        QRectF cell(rect);
        cell.setSize(QSizeF(header()->sectionSize(0), rect.height()));
        painter.drawLine(cell.topRight() + QPointF(-0.5, 0.5),
                         cell.bottomRight() + QPointF(-0.5, -0.5));

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                         rect.topRight() - QPointF(0.5, 0.0));
        painter.drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                         rect.bottomRight() - QPointF(0.5, 0.0));
    }

    QTreeWidget::paintEvent(event);
}
