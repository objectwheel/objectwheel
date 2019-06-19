#include <toolboxdelegate.h>
#include <QTreeView>
#include <QPainter>

ToolboxDelegate::ToolboxDelegate(QTreeView* parent) : QStyledItemDelegate(parent)
  , m_view(parent)
{
}

void ToolboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                            const QModelIndex& index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);

    // Only top-level items
    if (!model->parent(index).isValid()) {
        painter->save();

        // Only draw top line if the previous item is expanded
        QRectF rect(option.rect);
        QModelIndex previousIndex = model->index(index.row() - 1, index.column());
        bool drawTopline = index.row() > 0 && m_view->isExpanded(previousIndex);
        qreal highlightOffset = drawTopline ? 0 : -0.5;

        painter->setClipRect(rect);
        painter->setClipping(true);

        // Fill background
        QLinearGradient gradient(0, 0, 0, 1);
        gradient.setCoordinateMode(QGradient::ObjectMode);
        gradient.setColorAt(0, QColor("#ececec"));
        gradient.setColorAt(1, QColor("#dddddd"));
        painter->fillRect(rect, gradient);

        // Draw top line
        painter->setPen("#f5f5f5");
        painter->drawLine(rect.topLeft() + QPointF(0.5, 0.5) + QPointF(0.0, highlightOffset),
                          rect.topRight() + QPointF(-0.5, 0.5) + QPointF(0.0, highlightOffset));
        if (drawTopline) {
            painter->setPen("#bfbfbf");
            painter->drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                              rect.topRight() + QPointF(-0.5, 0.0));
        }

        // Draw bottom line
        painter->setPen("#bdbdbd");
        painter->drawLine(rect.bottomLeft() + QPointF(0.5, -0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
        painter->setPen("#a7a7a7");
        painter->drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                          rect.bottomRight() + QPointF(-0.5, 0.0));

        // Draw collapse/expand indicator
        QStyleOption branchOption;
        branchOption.rect = QRect(rect.left() + 5, rect.top() + rect.height() / 2 - 5, 10, 10);
        branchOption.palette = option.palette;
        branchOption.state = QStyle::State_Children;
        if (m_view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;
        m_view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

        painter->restore();

        // Draw text
        QRect textRect(rect.left() + 20, rect.top(), rect.width() - 25, rect.height());
        QString text = elidedText(option.fontMetrics, textRect.width(), Qt::ElideMiddle,
                                  model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textRect, Qt::AlignCenter,
                                      option.palette, m_view->isEnabled(), text);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ToolboxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index) + QSize(2, 2);
}
