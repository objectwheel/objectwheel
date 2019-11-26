#include <propertiestree.h>
#include <propertiesdelegate.h>
#include <designerscene.h>
#include <private/qtreewidget_p.h>

#include <QHeaderView>
#include <QPainter>

PropertiesTree::PropertiesTree(QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(nullptr)
  , m_delegate(new PropertiesDelegate(this))
{
    m_delegate->reserveSmart();

    header()->setFixedHeight(20);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->resizeSection(0, 165); // Don't resize the last (stretched) column

    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(0, tr("Property"));

    setColumnCount(2);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(m_delegate);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto updatePalette = [=] {
        QPalette p(palette());
        p.setColor(QPalette::Light, "#ab8157");
        p.setColor(QPalette::Dark, "#9c7650");
        p.setColor(QPalette::AlternateBase, "#f7efe6");
        p.setColor(QPalette::Link, "#489c3b");
        setPalette(p);
        setStyleSheet(QString { R"qss(
                        QTreeView {
                            border: 1px solid %1;
                        } QHeaderView::section {
                            color: %4;
                            padding-left: 5px;
                            padding-top: 3px;
                            padding-bottom: 3px;
                            border-style: solid;
                            border-left-width: 0px;
                            border-top-width: 0px;
                            border-bottom-color: %1;
                            border-bottom-width: 1px;
                            border-right-color: %1;
                            border-right-width: 1px;
                            background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,
                                                        stop:0 %2, stop:1 %3);
                        }
                        QHeaderView::section:last{
                            border-left-width: 0px;
                            border-right-width: 0px;
                        }
                    )qss"}
                    .arg(palette().dark().color().darker(120).name())
                    .arg(palette().light().color().name())
                    .arg(palette().dark().color().name())
                    .arg(palette().brightText().color().name()));
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();
}

void PropertiesTree::setDesignerScene(DesignerScene* designerScene)
{
    m_designerScene = designerScene;
}

PropertiesDelegate* PropertiesTree::delegate() const
{
    return m_delegate;
}

QList<QTreeWidgetItem*> PropertiesTree::topLevelItems() const
{
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < topLevelItemCount(); ++i)
        items.append(topLevelItem(i));
    return items;
}

QList<QTreeWidgetItem*> PropertiesTree::allSubChildItems(QTreeWidgetItem* parentItem,
                                                         bool includeParent,
                                                         bool includeCollapsed,
                                                         bool includeHidden) const
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if ((!includeCollapsed && !parentItem->isExpanded())
            || (!includeHidden && parentItem->isHidden())) {
        if (includeParent && (includeHidden || !parentItem->isHidden()))
            items.append(parentItem);

        return items;
    }

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        if (includeHidden || !parentItem->child(i)->isHidden())
            items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false, includeCollapsed, includeHidden));
    }

    return items;
}

void PropertiesTree::paintEvent(QPaintEvent* event)
{
    Q_D(const QTreeWidget);

    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());

    QColor lineColor(palette().dark().color());
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    qreal height = d->defaultItemHeight > 0 ? d->defaultItemHeight : 20;
    qreal rowCount = viewport()->height() / height;
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    for (int i = 0; i < rowCount; ++i) {
        painter.save();
        QRectF rect(0, i * height, viewport()->width(), height);
        painter.setClipRect(rect);

        if (i % 2) {
            painter.fillRect(rect, palette().alternateBase());
        } else if (topLevelItemCount() == 4) { // WARNING: Number of permanent items
            if (i == int((rowCount - 1) / 2.0) || i == int((rowCount - 1)/ 2.0) + 1) {
                QString message;
                if (selectedControls.size() == 0)
                    message = tr("No controls selected");
                else if (selectedControls.size() == 1)
                    message = tr("The control has got errors");
                else
                    message = tr("Multiple controls selected");

                QColor messageColor = palette().dark().color();
                messageColor.setAlpha(180);

                painter.setPen(messageColor);
                painter.drawText(rect, Qt::AlignCenter, message);
                painter.setPen(lineColor);
            }
        }

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
        painter.restore();
    }
    painter.end();

    QTreeWidget::paintEvent(event);
}

void PropertiesTree::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->save();

    const qreal width = 10;
    const QAbstractItemModel* model = index.model();
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 3; // NOTE: For Temporary "index" entry, should be 2 otherwise

    const QRectF r(rect);
    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(r.center());
    handleRect.moveRight(r.right() - 0.5);

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;

    m_delegate->paintBackground(painter, option,
                                m_delegate->calculateVisibleRow(itemFromIndex(index)),
                                isClassRow, false);

    // Draw handle
    if (hasChild) {
        const QColor c = isClassRow ? palette().brightText().color() : palette().text().color();
        painter->setPen(QPen(c, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);
        painter->drawLine(QPointF(handleRect.left() + 2, handleRect.center().y()),
                          QPointF(handleRect.right() - 2, handleRect.center().y()));
        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2));
        }
    }

    painter->restore();
}
