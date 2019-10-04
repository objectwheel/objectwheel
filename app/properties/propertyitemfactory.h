#ifndef PROPERTYITEMFACTORY_H
#define PROPERTYITEMFACTORY_H

#include <QHash>
#include <QObject>

class QTreeWidgetItem;
class QWidget;
template<class T>
class QStack;

class PropertyItemFactory final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyItemFactory)

public:
    enum Type {
        String,
        Url,
        Enum,
        Bool,
        Color,
        NumberInt,
        NumberReal,
        FontFamily,
        FontWeight,
        FontSize,
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
    explicit PropertyItemFactory(QObject* parent = nullptr);
    ~PropertyItemFactory() override;

    void clear();
    void reserve(int size);
    void push(Type type, const PropertyItem& propertyItem);
    PropertyItem pop(Type type);

private:
    PropertyItem createPropertyItem(Type type);

private:
    PropertyHash m_items;
};

#endif // PROPERTYITEMFACTORY_H
