#ifndef TOOLBOXDELEGATE_H
#define TOOLBOXDELEGATE_H

#include <QStyledItemDelegate>

class QTreeView;
class ToolboxDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxDelegate)

public:
    explicit ToolboxDelegate(QTreeView* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    QTreeView* m_view;
};

#endif // TOOLBOXDELEGATE_H
