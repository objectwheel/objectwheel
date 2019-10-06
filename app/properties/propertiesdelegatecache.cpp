#include <propertiesdelegatecache.h>
#include <propertiesdelegate.h>
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

PropertiesDelegateCache::PropertiesDelegateCache(QObject* parent) : QObject(parent)
{
}

PropertiesDelegateCache::~PropertiesDelegateCache()
{
    clear();
}

void PropertiesDelegateCache::clear()
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

void PropertiesDelegateCache::reserve(int size)
{
    const QMetaEnum& e = QMetaEnum::fromType<PropertiesDelegate::Type>();
    for (int i = 0; i < e.keyCount(); ++i) {
        PropertiesDelegate::Type type = PropertiesDelegate::Type(e.value(i));
        if (type == PropertiesDelegate::Type::Invalid)
            continue;
        for (int i = size; i--;)
            push(type, createWidget(type));
    }
    for (int i = size; i--;)
        push(new QTreeWidgetItem);
}

void PropertiesDelegateCache::push(int type, QWidget* widget)
{
    Q_ASSERT(widget);
    Q_ASSERT(type != PropertiesDelegate::Type::Invalid);
    if (!m_widgets.contains(type))
        m_widgets.insert(type, new WidgetStack);
    widget->setParent(nullptr);
    widget->setVisible(false);
    m_widgets.value(type)->push(widget);
}

void PropertiesDelegateCache::push(QTreeWidgetItem* item)
{
    if (QTreeWidgetItem* parent = item->parent())
        parent->takeChild(parent->indexOfChild(item));
    else if (QTreeWidget* tree = item->treeWidget())
        tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));
    item->setHidden(true);
    m_items.push(item);
}

QWidget* PropertiesDelegateCache::pop(int type)
{
    Q_ASSERT(type != PropertiesDelegate::Type::Invalid);
    if (WidgetStack* stack = m_widgets.value(type, nullptr)) {
        if (!stack->isEmpty())
            return stack->pop();
    }
    return createWidget(type);
}

QTreeWidgetItem* PropertiesDelegateCache::pop()
{
    if (m_items.isEmpty())
        return new QTreeWidgetItem;
    return m_items.pop();
}

QWidget* PropertiesDelegateCache::createWidget(int type)
{
    switch (type) {
    case PropertiesDelegate::String: {
        auto lineEdit = new QLineEdit;
        lineEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
        lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
        lineEdit->setFocusPolicy(Qt::StrongFocus);
        lineEdit->setSizePolicy(QSizePolicy::Ignored, lineEdit->sizePolicy().verticalPolicy());
        lineEdit->setMinimumWidth(1);
        return lineEdit;
    }

    case PropertiesDelegate::Enum: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::ClickFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        return comboBox;
    }

    case PropertiesDelegate::Bool: {
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

    case PropertiesDelegate::Color: {
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

    case PropertiesDelegate::Int: {
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

    case PropertiesDelegate::Real: {
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

    case PropertiesDelegate::FontSize: {
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

    case PropertiesDelegate::FontFamily: {
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

    case PropertiesDelegate::FontWeight: {
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

    case PropertiesDelegate::FontCapitalization: {
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
