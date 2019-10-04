#include <propertyitemcache.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>

#include <QMetaEnum>
#include <QStack>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QToolButton>
#include <QSpinBox>
#include <QFontDatabase>

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
    PropertyItem propertyItem;
    propertyItem.item = new QTreeWidgetItem;

    switch (type) {
    case String: {
        auto lineEdit = new QLineEdit;
        lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
        lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        lineEdit->setFocusPolicy(Qt::StrongFocus);
        lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
        lineEdit->setMinimumWidth(1);
        propertyItem.widget = lineEdit;
        break;
    }

    case Enum: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        propertyItem.widget = comboBox;
        break;
    }

    case Bool: {
        auto checkBox = new QCheckBox;
        checkBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        checkBox->setCursor(Qt::PointingHandCursor);
        checkBox->setFocusPolicy(Qt::ClickFocus);
        checkBox->setMinimumWidth(1);
        auto widget = new QWidget;
        widget->setMinimumWidth(1);
        widget->setAttribute(Qt::WA_MacShowFocusRect, false);
        widget->setFocusPolicy(Qt::ClickFocus);
        widget->setSizePolicy(QSizePolicy::Ignored, widget->sizePolicy().verticalPolicy());
        auto layout = new QHBoxLayout(widget);
        layout->addWidget(checkBox);
        layout->addStretch();
        layout->setSpacing(0);
        layout->setContentsMargins(2, 0, 0, 0);
        propertyItem.widget = widget;
        break;
    }

    case Color: {
        auto toolButton = new QToolButton;
        toolButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setAttribute(Qt::WA_MacShowFocusRect, false);
        toolButton->setIconSize({12, 12});
        toolButton->setCursor(Qt::PointingHandCursor);
        toolButton->setFocusPolicy(Qt::ClickFocus);
        toolButton->setSizePolicy(QSizePolicy::Ignored, toolButton->sizePolicy().verticalPolicy());
        toolButton->setMinimumWidth(1);
        propertyItem.widget = toolButton;
        break;
    }

    case Int: {
        auto spinBox = new QSpinBox;
        spinBox->setCursor(Qt::PointingHandCursor);
        spinBox->setFocusPolicy(Qt::StrongFocus);
        spinBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        spinBox->setSizePolicy(QSizePolicy::Ignored, spinBox->sizePolicy().verticalPolicy());
        spinBox->setMaximum(std::numeric_limits<int>::max());
        spinBox->setMinimum(std::numeric_limits<int>::lowest());
        spinBox->setMinimumWidth(1);
        TransparentStyle::attach(spinBox);
        UtilityFunctions::disableWheelEvent(spinBox);
        propertyItem.widget = spinBox;
        break;
    }

    case Real: {
        auto spinBox = new QDoubleSpinBox;
        spinBox->setCursor(Qt::PointingHandCursor);
        spinBox->setFocusPolicy(Qt::StrongFocus);
        spinBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        spinBox->setSizePolicy(QSizePolicy::Ignored, spinBox->sizePolicy().verticalPolicy());
        spinBox->setMaximum(std::numeric_limits<qreal>::max());
        spinBox->setMinimum(std::numeric_limits<qreal>::lowest());
        spinBox->setMinimumWidth(1);
        TransparentStyle::attach(spinBox);
        UtilityFunctions::disableWheelEvent(spinBox);
        propertyItem.widget = spinBox;
        break;
    }

    case FontSize: {
        auto spinBox = new QSpinBox;
        spinBox->setCursor(Qt::PointingHandCursor);
        spinBox->setFocusPolicy(Qt::StrongFocus);
        spinBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        spinBox->setMinimum(0);
        spinBox->setMaximum(9999);
        spinBox->setSizePolicy(QSizePolicy::Ignored, spinBox->sizePolicy().verticalPolicy());
        spinBox->setMinimumWidth(1);
        TransparentStyle::attach(spinBox);
        UtilityFunctions::disableWheelEvent(spinBox);
        propertyItem.widget = spinBox;
        break;
    }

    case FontFamily: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        propertyItem.widget = comboBox;
        comboBox->addItems(QFontDatabase().families());
        break;
    }

    case FontWeight: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        propertyItem.widget = comboBox;
        auto e = QMetaEnum::fromType<QFont::Weight>();
        for (int i = 0; i < e.keyCount(); ++i)
            comboBox->addItem(e.key(i));
        break;
    }

    case FontCapitalization: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        propertyItem.widget = comboBox;
        auto e = QMetaEnum::fromType<QFont::Capitalization>();
        for (int i = 0; i < e.keyCount(); ++i)
            comboBox->addItem(e.key(i));
        break;
    }
    }

    return propertyItem;
}
