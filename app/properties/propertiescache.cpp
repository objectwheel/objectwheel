#include <propertiescache.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>

#include <QTreeWidgetItem>
#include <QMetaEnum>
#include <QStack>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QToolButton>
#include <QSpinBox>
#include <QFontDatabase>

PropertiesCache::PropertiesCache(QObject* parent) : QObject(parent)
{
}

PropertiesCache::~PropertiesCache()
{
    clear();
}

void PropertiesCache::clear()
{
    for (WidgetStack* stack : qAsConst(m_widgets)) {
        const WidgetStack& _stack = *stack;
        for (QWidget* widget : _stack)
            delete widget;
        delete stack;
    }
    m_widgets.clear();

    for (QTreeWidgetItem* item : qAsConst(m_items))
        delete item;
    m_items.clear();
}

void PropertiesCache::reserve(int size)
{
    const QMetaEnum& e = QMetaEnum::fromType<Type>();
    for (int i = 0; i < e.keyCount(); ++i) {
        Type type = Type(e.value(i));
        if (type == Type::Invalid)
            continue;
        for (int i = size; i--;)
            push(type, createWidget(type));
    }
    for (int i = size; i--;)
        push(new QTreeWidgetItem);
}

void PropertiesCache::push(PropertiesCache::Type type, QWidget* widget)
{
    Q_ASSERT(widget);
    Q_ASSERT(type != Type::Invalid);
    if (!m_widgets.contains(type))
        m_widgets.insert(type, new WidgetStack);
    widget->setParent(nullptr);
    widget->setVisible(false);
    m_widgets.value(type)->push(widget);
}

void PropertiesCache::push(QTreeWidgetItem* item)
{
    if (QTreeWidgetItem* parent = item->parent())
        parent->takeChild(parent->indexOfChild(item));
    else if (QTreeWidget* tree = item->treeWidget())
        tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));
    item->setHidden(true);
    m_items.push(item);
}

QWidget* PropertiesCache::pop(PropertiesCache::Type type)
{
    Q_ASSERT(type != Type::Invalid);
    if (WidgetStack* stack = m_widgets.value(type, nullptr)) {
        if (!stack->isEmpty())
            return stack->pop();
    }
    return createWidget(type);
}

QTreeWidgetItem* PropertiesCache::pop()
{
    if (m_items.isEmpty())
        return new QTreeWidgetItem;
    return m_items.pop();
}

QWidget* PropertiesCache::createWidget(PropertiesCache::Type type)
{
    switch (type) {
    case String: {
        auto lineEdit = new QLineEdit;
        lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
        lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        lineEdit->setFocusPolicy(Qt::StrongFocus);
        lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
        lineEdit->setMinimumWidth(1);
        return lineEdit;
    }

    case Enum: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        return comboBox;
    }

    case Bool: {
        auto checkBox = new QCheckBox;
        checkBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        checkBox->setCursor(Qt::PointingHandCursor);
        checkBox->setFocusPolicy(Qt::ClickFocus);
        checkBox->setSizePolicy(QSizePolicy::Ignored, checkBox->sizePolicy().verticalPolicy());
        checkBox->setMinimumWidth(1);
        checkBox->setFixedWidth(checkBox->style()->pixelMetric(QStyle::PM_IndicatorWidth) + 3);
        checkBox->setStyleSheet("QCheckBox { margin-left: 2px; }");
        return checkBox;
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
        return toolButton;
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
        return spinBox;
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
        return spinBox;
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
        return spinBox;
    }

    case FontFamily: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        comboBox->addItems(QFontDatabase().families());
        return comboBox;
    }

    case FontWeight: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        const QMetaEnum& e = QMetaEnum::fromType<QFont::Weight>();
        for (int i = 0; i < e.keyCount(); ++i)
            comboBox->addItem(e.key(i));
        return comboBox;
    }

    case FontCapitalization: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        const QMetaEnum& e = QMetaEnum::fromType<QFont::Capitalization>();
        for (int i = 0; i < e.keyCount(); ++i)
            comboBox->addItem(e.key(i));
        return comboBox;
    }

    default:
        break;
    }

    return nullptr;
}
