#ifndef PROPERTYITEMCACHE_H
#define PROPERTYITEMCACHE_H

#include <QHash>
#include <QObject>

class QTreeWidgetItem;
class QWidget;
template<class T>
class QStack;

class PropertyItemCache final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyItemCache)

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

    struct PropertyItem {
        QTreeWidgetItem* item;
        QWidget* widget;
    };

    using PropertyStack = QStack<PropertyItem>;
    using PropertyHash = QHash<Type, PropertyStack*>;

public:
    explicit PropertyItemCache(QObject* parent = nullptr);
    ~PropertyItemCache() override;

    void clear();
    void reserve(int size);
    void push(Type type, const PropertyItem& propertyItem);
    PropertyItem pop(Type type);

private:
    PropertyItem createPropertyItem(Type type);

private:
    PropertyHash m_items;
};

#endif // PROPERTYITEMCACHE_H
