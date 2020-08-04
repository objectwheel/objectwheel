#ifndef STYLEDITEMDELEGATE_H
#define STYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QAbstractItemView>

class StyleOptionViewItem final : public QStyleOptionViewItem
{
public:
    StyleOptionViewItem(const QStyleOptionViewItem& other);
    StyleOptionViewItem& operator=(const QStyleOptionViewItem& other);
    const QAbstractItemView* view = nullptr;
};

class StyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(StyledItemDelegate)

public:
    explicit StyledItemDelegate(QObject* parent = nullptr);

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

#endif // STYLEDITEMDELEGATE_H
