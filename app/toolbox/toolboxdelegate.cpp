#include <toolboxdelegate.h>
#include <QTreeView>
#include <QPainter>

ToolboxDelegate::ToolboxDelegate(QTreeView* parent) : QStyledItemDelegate(parent)
  , m_view(parent)
{
}

void ToolboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    if (!model->parent(index).isValid()) {
        // this is a top-level item.
        painter->save();

        // Only draw topline if the previous item is expanded
        QModelIndex previousIndex = model->index(index.row() - 1, index.column());
        bool drawTopline = (index.row() > 0 && m_view->isExpanded(previousIndex));

        auto frame = option.rect;

        QLinearGradient gradient(frame.topLeft(), frame.bottomLeft());
        gradient.setColorAt(0, QColor("#fafafa"));
        gradient.setColorAt(1, QColor("#e3e3e3"));
        painter->setPen(Qt::NoPen);
        painter->setBrush(gradient);
        painter->drawRect(frame);

        painter->setPen("#d0d0d0");
        if (drawTopline)
            painter->drawLine(frame.topLeft(), frame.topRight());
        painter->drawLine(frame.bottomLeft(), frame.bottomRight());
        painter->restore();

        QStyleOption branchOption;
        static const int i = 9; // ### hardcoded in qcommonstyle.cpp
        QRect r = frame;
        branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
        branchOption.state = QStyle::State_Children;

        if (m_view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;

        m_view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

        // draw text
        QRect textrect = QRect(r.left() + i*2, r.top(), r.width() - ((5*i)/2), r.height());
        QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle,
                                  model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
                                      option.palette, m_view->isEnabled(), text);

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ToolboxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index) + QSize(2, 2);
}
