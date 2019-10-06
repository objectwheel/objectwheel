#include <propertiesdelegate.h>
#include <propertiestree.h>
#include <propertiesdelegatecache.h>
#include <transparentstyle.h>

#include <QPainter>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QMetaEnum>
#include <QTreeWidgetItem>
#include <QMetaEnum>
#include <QStack>
#include <QToolButton>
#include <QSpinBox>
#include <QFontDatabase>

static void setValues(QWidget* widget, PropertiesDelegate::Type type, const QVariant& value)
{
    switch (type) {
    case PropertiesDelegate::Enum:
    case PropertiesDelegate::FontFamily:
    case PropertiesDelegate::FontWeight:
    case PropertiesDelegate::FontCapitalization:
        static_cast<QComboBox*>(widget)->addItems(value.value<QStringList>());
        break;
    default:
        break;
    }
}

static void setInitialValue(QWidget* widget, PropertiesDelegate::Type type, const QVariant& value)
{
    const char* propertyName = 0;
    switch (type) {
    case PropertiesDelegate::String:
        propertyName = "text";
        break;
    case PropertiesDelegate::Bool:
        propertyName = "checked";
        break;
    case PropertiesDelegate::Color:
        propertyName = "color";
        break;
    case PropertiesDelegate::Int:
    case PropertiesDelegate::Real:
    case PropertiesDelegate::FontSize:
        propertyName = "value";
        break;
    case PropertiesDelegate::Enum:
    case PropertiesDelegate::FontFamily:
    case PropertiesDelegate::FontWeight:
    case PropertiesDelegate::FontCapitalization:
        propertyName = "currentText";
        break;
    default:
        break;
    }
    if (propertyName)
        widget->setProperty(propertyName, value);
}

PropertiesDelegate::PropertiesDelegate(PropertiesTree* propertiesTree) : QStyledItemDelegate(propertiesTree)
  , m_propertiesTree(propertiesTree)
  , m_cache(new PropertiesDelegateCache)
{
}

PropertiesDelegate::~PropertiesDelegate()
{
    delete m_cache;
}

void PropertiesDelegate::reserve(int size)
{
    const QMetaEnum& e = QMetaEnum::fromType<Type>();
    for (int i = 0; i < e.keyCount(); ++i) {
        Type type = Type(e.value(i));
        if (type == Type::Invalid)
            continue;
        for (int i = size; i--;)
            m_cache->push(type, createWidget(type));
    }
    for (int i = size; i--;)
        m_cache->push(new QTreeWidgetItem);
}

int PropertiesDelegate::calculateVisibleRow(const QTreeWidgetItem* item) const
{
    const QList<QTreeWidgetItem*>& topLevelItems = m_propertiesTree->topLevelItems();
    int totalCount = 0;
    for (QTreeWidgetItem* topLevelItem : topLevelItems)
        totalCount += m_propertiesTree->allSubChildItems(topLevelItem, true, false).size();

    int count = 0;
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        for (QTreeWidgetItem* childItem : m_propertiesTree->allSubChildItems(topLevelItem, true, false)) {
            if (childItem == item)
                return totalCount - count - 1;
            ++count;
        }
    }

    Q_ASSERT(0);
    return 0;
}

void PropertiesDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                         int rowNumber, bool isClassRow, bool hasVerticalLine) const
{
    painter->save();

    const QRectF& rect = option.rect;

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (isClassRow)
        painter->fillRect(rect, option.palette.light());
    else if (rowNumber % 2)
        painter->fillRect(rect, option.palette.alternateBase());
    else
        painter->fillRect(rect, option.palette.base());

    // Draw top and bottom lines
    QColor lineColor(option.palette.dark().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF(0.5, 0.0),
                      rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
                      rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (hasVerticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

void PropertiesDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const QAbstractItemModel* model = index.model();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 3;

    paintBackground(painter, option, calculateVisibleRow(m_propertiesTree->itemFromIndex(index)),
                    isClassRow, index.column() == 0 && !isClassRow);

    // Draw text
    if (isClassRow) {
        painter->setPen(option.palette.highlightedText().color());
    } else {
        if (index.column() == 0 && index.data(ModificationRole).toBool()) {
            QFont font (option.font);
            font.setWeight(QFont::Medium);
            painter->setFont(font);
            painter->setPen(option.palette.link().color());
        } else {
            painter->setPen(option.palette.text().color());
        }
    }

    const QRectF& textRect = option.rect.adjusted(5, 0, 0, 0);
    const QString& text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect, option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                      QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    painter->restore();
}

QSize PropertiesDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QStyledItemDelegate::sizeHint(option, index).width(), ROW_HEIGHT);
}

void PropertiesDelegate::setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const
{
    // do nothing
}

void PropertiesDelegate::setEditorData(QWidget*, const QModelIndex&) const
{
    // do nothing
}

QWidget* PropertiesDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    if (index.column() == 0)
        return nullptr;

    auto type = index.data(TypeRole).value<Type>();
    if (type == Invalid)
        return QStyledItemDelegate::createEditor(parent, option, index);

    QWidget* widget = m_cache->pop(type);
    if (widget == 0)
        widget = createWidget(type);
    widget->setParent(parent);
    widget->setVisible(true);
    setValues(widget, type, index.data(ValuesRole));
    setInitialValue(widget, type, index.data(InitialValueRole));
    addConnection(widget, type, index.data(PropertyNameRole).toString());
    return widget;
}

void PropertiesDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() == 0)
        return editor->deleteLater();

    auto type = index.data(TypeRole).value<Type>();
    if (type == Invalid)
        return QStyledItemDelegate::destroyEditor(editor, index);

    clearConnection(editor);
    m_cache->push(type, editor);
}

QTreeWidgetItem* PropertiesDelegate::createItem() const
{
    if (QTreeWidgetItem* item = m_cache->pop())
        return item;
    return new QTreeWidgetItem;
}

void PropertiesDelegate::destroyItem(QTreeWidgetItem* item) const
{
    m_cache->push(item);
}

void PropertiesDelegate::addConnection(QWidget* widget, int type, const QString& propertyName) const
{
    switch (type) {
    case String: {
        auto lineEdit = static_cast<QLineEdit*>(widget);
        connect(lineEdit, &QLineEdit::editingFinished,
                [=] { emit propertyEdited(type, propertyName, lineEdit->text()); });
    } break;
    case Enum: {
        auto comboBox = static_cast<QComboBox*>(widget);
        connect(comboBox, qOverload<int>(&QComboBox::activated),
                [=] { emit propertyEdited(type, propertyName, comboBox->currentText()); });
    } break;
    case Bool: {
        auto checkBox = static_cast<QCheckBox*>(widget);
        connect(checkBox, qOverload<bool>(&QCheckBox::clicked),
                [=] { emit propertyEdited(type, propertyName, checkBox->isChecked()); });
    } break;
//    case Color:
//        propertyName = "color";
//        break;
//    case Int:
//        propertyName = "value";
//        break;
//    case Real:
//        propertyName = "value";
//        break;
//    case FontSize:
//        propertyName = "value";
//        break;
//    case FontFamily:
//        propertyName = "currentText";
//        break;
//    case FontWeight:
//        propertyName = "currentText";
//        break;
//    case FontCapitalization:
//        propertyName = "currentText";
//        break;
    default:
        break;
    }
}

QWidget* PropertiesDelegate::createWidget(int type) const
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

void PropertiesDelegate::clearConnection(QWidget* widget) const
{
    disconnect(widget, 0, 0, 0);
}
