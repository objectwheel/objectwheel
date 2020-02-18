#include <styleditemdelegate.h>

StyledItemDelegate::StyledItemDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void StyledItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    // Default implementation messes with the decorationSize.
    // It replaces the default value (QAbstractItemView::iconSize)
    // with the actual icon size (which we don't want)
    const QSize decorationSize = option->decorationSize;
    QStyledItemDelegate::initStyleOption(option, index);
    if (decorationSize.isValid())
        option->decorationSize = decorationSize;
}
