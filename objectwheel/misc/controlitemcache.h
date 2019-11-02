#ifndef CONTROLITEMCACHE_H
#define CONTROLITEMCACHE_H

#include <QStack>
#include <QTreeWidgetItem>

class ControlItem final : public QTreeWidgetItem
{
public:
    enum Roles { ControlRole = Qt::UserRole + 1 };
    bool operator<(const QTreeWidgetItem& other) const;
};

class ControlItemCache final
{
public:
    ControlItemCache() = default;
    ControlItemCache(const ControlItemCache&) = delete;
    ControlItemCache& operator=(const ControlItemCache&) = delete;
    ~ControlItemCache();

    void push(QTreeWidgetItem* item);
    QTreeWidgetItem* pop();

private:
    QStack<QTreeWidgetItem*> m_items;
};

#endif // CONTROLITEMCACHE_H
