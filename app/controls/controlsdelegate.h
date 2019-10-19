#ifndef CONTROLSDELEGATE_H
#define CONTROLSDELEGATE_H

#include <QStyledItemDelegate>

class QTreeWidgetItem;
class ControlsTree;
class ControlsDelegateCache;
class Control;

class ControlsDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlsDelegate)

    friend class ControlsTree; // For paintBackground() and calculateVisibleRow()
    friend class ControlsController; // For createItem() and destroyItem()

public:
    enum Roles {
        ControlRole = Qt::UserRole + 1
    };

public:
    explicit ControlsDelegate(ControlsTree* controlsTree);
    ~ControlsDelegate() override;

    void reserve();

private:
    void destroyItem(QTreeWidgetItem* item) const;
    QTreeWidgetItem* createItem(Control* control) const;

    int calculateVisibleRow(const QTreeWidgetItem* item) const;
    void paintBackground(QPainter* painter, const QStyleOptionViewItem& option, int rowNumber,
                         bool hasVerticalLine) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
private:
    ControlsTree* m_controlsTree;
    ControlsDelegateCache* m_cache;
};

#define EVERYTHING(variable, tree)                                    \
    Q_FOREACH(QTreeWidgetItem* _topLevelItem_, tree->topLevelItems()) \
    Q_FOREACH(variable, tree->allSubChildItems(_topLevelItem_))

#endif // CONTROLSDELEGATE_H
