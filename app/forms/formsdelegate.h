#ifndef FORMSDELEGATE_H
#define FORMSDELEGATE_H

#include <QStyledItemDelegate>

class FormsTree;
class FormsDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(FormsDelegate)

public:
    enum Roles {
        ControlRole = Qt::UserRole + 1
    };

public:
    explicit FormsDelegate(FormsTree* formsTree);

private:
    void paintBackground(QPainter* painter, const QStyleOptionViewItem& option, int rowNumber) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    FormsTree* m_formsTree;
};

#define EVERYTHING(variable, tree) \
    Q_FOREACH(variable, tree->topLevelItems())

#endif // FORMSDELEGATE_H
