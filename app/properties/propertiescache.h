#ifndef PROPERTIESCACHE_H
#define PROPERTIESCACHE_H

#include <QHash>
#include <QStack>
#include <QObject>

class QWidget;
class QTreeWidgetItem;

class PropertiesCache final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesCache)

public:
    enum Type {
        String,
        Enum,
        Bool,
        Color,
        Int,
        Real,
        FontSize,
        FontFamily,
        FontWeight,
        FontCapitalization
    };
    Q_ENUM(Type)

public:
    explicit PropertiesCache(QObject* parent = nullptr);
    ~PropertiesCache() override;

    void clear();
    void reserve(int size);
    void push(Type type, QWidget* widget);
    void push(QTreeWidgetItem* item);
    QWidget* pop(Type type);
    QTreeWidgetItem* pop();

private:
    QWidget* createWidget(Type type);

private:
    using WidgetStack = QStack<QWidget*>;
    using WidgetHash = QHash<Type, WidgetStack*>;
    using ItemStack = QStack<QTreeWidgetItem*>;

    WidgetHash m_widgets;
    ItemStack m_items;
};

#endif // PROPERTIESCACHE_H
