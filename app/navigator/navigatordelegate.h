#ifndef NAVIGATORDELEGATE_H
#define NAVIGATORDELEGATE_H

#include <QStyledItemDelegate>

class QTreeWidgetItem;
class NavigatorTree;
class NavigatorDelegateCache;
class Control;

class NavigatorDelegate final : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(NavigatorDelegate)

    friend class NavigatorTree; // For paintBackground() and calculateVisibleRow()
    friend class NavigatorController; // For createItem() and destroyItem()

public:
    enum Roles {
        ControlRole = Qt::UserRole + 1
    };

public:
    explicit NavigatorDelegate(NavigatorTree* navigatorTree);
    ~NavigatorDelegate() override;

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
    NavigatorTree* m_navigatorTree;
    NavigatorDelegateCache* m_cache;
};

#endif // NAVIGATORDELEGATE_H
