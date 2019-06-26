#include <issueslistdelegate.h>
#include <QPainter>

IssuesListDelegate::IssuesListDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void IssuesListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->fillRect(option.rect, "#f0f0f0");
    painter->setPen("#c4c4c4");
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(option.rect.bottomLeft() + QPointF(0.5, 0.5),
                      option.rect.bottomRight() + QPointF(0.5, 0.5));
    QStyledItemDelegate::paint(painter, option, index);
}