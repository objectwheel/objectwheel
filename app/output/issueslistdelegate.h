#ifndef ISSUESLISTDELEGATE_H
#define ISSUESLISTDELEGATE_H

#include <QStyledItemDelegate>

class IssuesListDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(IssuesListDelegate)

public:
    explicit IssuesListDelegate(QObject* parent = nullptr);

private:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};


#endif // ISSUESLISTDELEGATE_H
