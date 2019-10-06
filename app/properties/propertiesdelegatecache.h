#ifndef PROPERTIESLISTCACHE_H
#define PROPERTIESLISTCACHE_H

#include <QHash>
#include <QStack>
#include <QObject>

class QWidget;
class QTreeWidgetItem;

class PropertiesDelegateCache final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesDelegateCache)

public:
    explicit PropertiesDelegateCache(QObject* parent = nullptr);
    ~PropertiesDelegateCache() override;

    void clear();
    void reserve(int size);
    void push(int type, QWidget* widget);
    void push(QTreeWidgetItem* item);
    QWidget* pop(int type);
    QTreeWidgetItem* pop();

private:
    QWidget* createWidget(int type);

private:
    using WidgetStack = QStack<QWidget*>;
    using WidgetHash = QHash<int, WidgetStack*>;
    using ItemStack = QStack<QTreeWidgetItem*>;

    WidgetHash m_widgets;
    ItemStack m_items;
};

#endif // PROPERTIESLISTCACHE_H
