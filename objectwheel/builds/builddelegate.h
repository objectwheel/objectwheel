#ifndef BUILDDELEGATE_H
#define BUILDDELEGATE_H

#include <styleditemdelegate.h>

class BuildDelegate final : public StyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildDelegate)

public:
    explicit BuildDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

#endif // BUILDDELEGATE_H
