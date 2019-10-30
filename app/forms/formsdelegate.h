#ifndef FORMSDELEGATE_H
#define FORMSDELEGATE_H

#include <QStyledItemDelegate>

class FormsTree;
class QTreeWidgetItem;
class ControlItemCache;
class Control;

class FormsDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(FormsDelegate)

    friend class FormsController; // For createItem() and destroyItem()

public:
    explicit FormsDelegate(FormsTree* formsTree);
    ~FormsDelegate() override;

    void reserve();

private:
    void destroyItem(QTreeWidgetItem* item) const;
    QTreeWidgetItem* createItem(Control* control) const;

    void paintBackground(QPainter* painter, const QStyleOptionViewItem& option, int rowNumber) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

private:
    FormsTree* m_formsTree;
    ControlItemCache* m_cache;
};

#define EVERYTHING(variable, tree) \
    Q_FOREACH(variable, tree->topLevelItems())

#endif // FORMSDELEGATE_H
