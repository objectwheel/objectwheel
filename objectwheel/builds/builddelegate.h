#ifndef BUILDDELEGATE_H
#define BUILDDELEGATE_H

#include <QStyledItemDelegate>

class QListView;
class BuildDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildDelegate)

public:
    explicit BuildDelegate(QListView* listView = nullptr);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    const QListView* m_listView;
};

#endif // BUILDDELEGATE_H
