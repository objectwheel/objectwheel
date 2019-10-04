#include <propertyitemfactory.h>
#include <QMetaEnum>
#include <QStack>
#include <QTreeWidgetItem>

PropertyItemFactory::PropertyItemFactory(QObject* parent) : QObject(parent)
{
}

PropertyItemFactory::~PropertyItemFactory()
{
    clear();
}

void PropertyItemFactory::clear()
{
    for (PropertyStack* stack : m_items) {
        for (PropertyItem propertyItem : *stack) {
            delete propertyItem.widget;
            delete propertyItem.item;
        }
        delete stack;
    }
    m_items.clear();
}

void PropertyItemFactory::reserve(int size)
{
    auto e = QMetaEnum::fromType<Type>();
    for (int i = 0; i < e.keyCount(); ++i) {
        Type type = Type(e.value(i));
        for (;size--;)
            push(type, createPropertyItem(type));
    }
}

void PropertyItemFactory::push(PropertyItemFactory::Type type,
                               const PropertyItemFactory::PropertyItem& propertyItem)
{
    if (!m_items.contains(type))
        m_items.insert(type, new PropertyStack);
    m_items.value(type)->push(propertyItem);
}

PropertyItemFactory::PropertyItem PropertyItemFactory::pop(PropertyItemFactory::Type type)
{
    if (PropertyStack* stack = m_items.value(type, nullptr)) {
        if (!stack->isEmpty())
            return stack->pop();
    }
    return createPropertyItem(type);
}

PropertyItemFactory::PropertyItem PropertyItemFactory::createPropertyItem(PropertyItemFactory::Type type)
{
//    String,
//    Url,
//    Enum,
//    Bool,
//    Color,
//    NumberInt,
//    NumberReal,
//    FontFamily,
//    FontWeight,
//    FontSize,
//    FontCapitalization
    PropertyItem propertyItem;

    switch (type) {
    case String: {
        propertyItem.item = new QTreeWidgetItem;
        item->setText(0, propertyName);
        item->setData(0, Qt::DecorationRole,
                      ParserUtils::exists(selectedControl->dir(), propertyName));
        classItem->addChild(item);
        m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
                                                          createStringHandlerWidget(propertyName, text, selectedControl));
        break;
    }

//    case Font: {
//        const QFont& font = propertyValue.value<QFont>();
//        createAndAddFontPropertiesBlock(classItem, font, selectedControl);
//        break;
//    }

//    case Color: {
//        const QColor& color = propertyValue.value<QColor>();
//        auto item = new QTreeWidgetItem;
//        item->setText(0, propertyName);
//        item->setData(0, Qt::DecorationRole,
//                      ParserUtils::exists(selectedControl->dir(), propertyName));
//        classItem->addChild(item);
//        m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
//                                                          createColorHandlerWidget(propertyName, color, selectedControl));
//        break;
//    }

//    case Bool: {
//        const bool checked = propertyValue.value<bool>();
//        auto item = new QTreeWidgetItem;
//        item->setText(0, propertyName);
//        item->setData(0, Qt::DecorationRole,
//                      ParserUtils::exists(selectedControl->dir(), propertyName));
//        classItem->addChild(item);
//        m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
//                                                          createBoolHandlerWidget(propertyName, checked, selectedControl));
//        break;
//    }

//    case Url: {
//        const QUrl& url = propertyValue.value<QUrl>();
//        const QString& displayText = urlToDisplayText(url, selectedControl->dir());
//        auto item = new QTreeWidgetItem;
//        item->setText(0, propertyName);
//        item->setData(0, Qt::DecorationRole,
//                      ParserUtils::exists(selectedControl->dir(), propertyName));
//        classItem->addChild(item);
//        m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
//                                                          createUrlHandlerWidget(propertyName, displayText, selectedControl));
//        break;
//    }

//    case Double: {
//        if (isXProperty(propertyName)) {
//            createAndAddGeometryPropertiesBlock(classItem, properties, selectedControl, false);
//        } else {
//            if (isGeometryProperty(propertyName))
//                break;

//            double number = propertyValue.value<double>();
//            auto item = new QTreeWidgetItem;
//            item->setText(0, propertyName);
//            item->setData(0, Qt::DecorationRole,
//                          ParserUtils::exists(selectedControl->dir(), propertyName));
//            classItem->addChild(item);
//            m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
//                                                              createNumberHandlerWidget(propertyName, number, selectedControl, false));
//        }
//        break;
//    }

//    case Int: {
//        if (isXProperty(propertyName)) {
//            createAndAddGeometryPropertiesBlock(classItem, properties, selectedControl, true);
//        } else {
//            if (isGeometryProperty(propertyName))
//                break;

//            int number = propertyValue.value<int>();
//            auto item = new QTreeWidgetItem;
//            item->setText(0, propertyName);
//            item->setData(0, Qt::DecorationRole,
//                          ParserUtils::exists(selectedControl->dir(), propertyName));
//            classItem->addChild(item);
//            m_propertiesPane->propertiesTree()->setItemWidget(item, 1,
//                                                              createNumberHandlerWidget(propertyName, number, selectedControl, true));
//        }
//        break;
//    }

    default:
        break;
    }
}
