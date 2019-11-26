#include <propertiesdelegate.h>
#include <propertiestree.h>
#include <propertiesdelegatecache.h>
#include <transparentstyle.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QPainter>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QMetaEnum>
#include <QToolButton>
#include <QSpinBox>
#include <QFontDatabase>

static const char typeProperty[] = "_q_PropertiesDelegate_type";

PropertiesDelegate::PropertiesDelegate(PropertiesTree* propertiesTree) : QStyledItemDelegate(propertiesTree)
  , m_propertiesTree(propertiesTree)
  , m_cache(new PropertiesDelegateCache)
{
}

PropertiesDelegate::~PropertiesDelegate()
{
    delete m_cache;
}

void PropertiesDelegate::reserveSmart()
{
    const QMetaEnum& e = QMetaEnum::fromType<Type>();
    for (int i = 0; i < e.keyCount(); ++i) {
        Type type = Type(e.value(i));
        if (type == Type::Invalid)
            continue;
        for (int i = smartSize(type); i--;) {
            QWidget* widget = createWidget(type);
            widget->setVisible(false);
            m_cache->push(type, widget);
        }
    }
    for (int i = 110; i--;) {
        auto item = new QTreeWidgetItem;
        item->setHidden(true);
        m_cache->push(item);
    }
}

void PropertiesDelegate::setEditorData(QWidget*, const QModelIndex&) const
{
    // do nothing
}

void PropertiesDelegate::setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const
{
    // do nothing
}

void PropertiesDelegate::destroyItem(QTreeWidgetItem* item) const
{
    if (QTreeWidgetItem* parent = item->parent())
        parent->takeChild(parent->indexOfChild(item));
    else if (QTreeWidget* tree = item->treeWidget())
        tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));
    item->setHidden(true);
    item->setText(0, QString());
    item->setText(1, QString());
    item->setData(0, PropertiesDelegate::ModificationRole, QVariant());
    item->setData(1, PropertiesDelegate::ValuesRole, QVariant());
    item->setData(1, PropertiesDelegate::InitialValueRole, QVariant());
    item->setData(1, PropertiesDelegate::TypeRole, QVariant());
    item->setData(1, PropertiesDelegate::CallbackRole, QVariant());
    m_cache->push(item);
}

void PropertiesDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() == 0)
        return editor->deleteLater();

    // Using typeProperty because the item is invalidated and
    // index.data(TypeRole) is invalid already at this point
    auto type = editor->property(typeProperty).value<Type>();
    if (type == Invalid)
        return QStyledItemDelegate::destroyEditor(editor, index);

    disconnect(editor, 0, 0, 0);
    clearWidget(editor, type);
    editor->setParent(nullptr);
    editor->setVisible(false);
    m_cache->push(type, editor);
}

QTreeWidgetItem* PropertiesDelegate::createItem() const
{
    if (QTreeWidgetItem* item = m_cache->pop())
        return item;
    return new QTreeWidgetItem;
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
    widget->setProperty(typeProperty, type);
    setValues(widget, type, index.data(ValuesRole));
    setInitialValue(widget, type, index.data(InitialValueRole));
    setConnection(widget, type, index.data(CallbackRole).value<Callback>());
    return widget;
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

    painter->setClipRect(rect);

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

    const QAbstractItemModel* model = index.model();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 3;

    paintBackground(painter, option, calculateVisibleRow(m_propertiesTree->itemFromIndex(index)),
                    isClassRow, index.column() == 0 && !isClassRow);

    // Draw text
    if (isClassRow) {
        painter->setPen(option.palette.brightText().color());
    } else {
        if (index.column() == 0 && index.data(ModificationRole).toBool())
            painter->setPen(option.palette.link().color());
        else
            painter->setPen(option.palette.text().color());
    }

    const QRectF& textRect = option.rect.adjusted(3, 0, 0, 0);
    const QString& text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect, option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                      QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    painter->restore();
}

void PropertiesDelegate::setValues(QWidget* widget, PropertiesDelegate::Type type, const QVariant& value) const
{
    switch (type) {
    case Enum:
        static_cast<QComboBox*>(widget)->addItems(value.value<QStringList>());
        break;
    default:
        break;
    }
}

void PropertiesDelegate::setInitialValue(QWidget* widget, PropertiesDelegate::Type type, const QVariant& value) const
{
    const char* propertyName = 0;
    switch (type) {
    case Url:
    case String:
        propertyName = "text";
        break;
    case Bool:
        propertyName = "checked";
        break;
    case Color: {
        const QColor& color = value.value<QColor>();
        auto toolButton = static_cast<QToolButton*>(widget);
        if (color.isValid()) {
            toolButton->setText(color.name(QColor::HexArgb));
            toolButton->setIcon(QIcon(PaintUtils::renderPropertyColorPixmap({12, 12}, color, {Qt::black},
                                                                            toolButton->devicePixelRatioF())));
        } else {
            toolButton->setText("<unassigned>");
            toolButton->setIcon(QIcon(PaintUtils::renderPropertyColorPixmap({12, 12}, Qt::transparent, {Qt::black},
                                                                            toolButton->devicePixelRatioF())));
        }
    } break;
    case Int:
    case Real:
    case FontSize:
        propertyName = "value";
        break;
    case Enum:
    case FontFamily:
    case FontWeight:
    case FontCapitalization:
        propertyName = "currentText";
        break;
    default:
        break;
    }
    if (propertyName)
        widget->setProperty(propertyName, value);
}

void PropertiesDelegate::setConnection(QWidget* widget, PropertiesDelegate::Type type, PropertiesDelegate::Callback callback) const
{
    switch (type) {
    case Url:
    case String: {
        auto lineEdit = static_cast<QLineEdit*>(widget);
        QObject::connect(lineEdit, &QLineEdit::editingFinished,
                         [=] { callback.call(lineEdit->text()); });
    } break;
    case Bool: {
        auto checkBox = static_cast<QCheckBox*>(widget);
        QObject::connect(checkBox, qOverload<bool>(&QCheckBox::clicked),
                         [=] { callback.call(checkBox->isChecked()); });
    } break;
    case FontFamily:
    case FontWeight:
    case FontCapitalization:
    case Enum: {
        auto comboBox = static_cast<QComboBox*>(widget);
        QObject::connect(comboBox, qOverload<int>(&QComboBox::activated),
                         [=] { callback.call(comboBox->currentText()); });
    } break;
    case FontSize:
    case Int: {
        auto spinBox = static_cast<QSpinBox*>(widget);
        QObject::connect(spinBox, qOverload<int>(&QSpinBox::valueChanged),
                         [=] { callback.call(QVariant::fromValue<QSpinBox*>(spinBox)); });
    } break;
    case Real: {
        auto spinBox = static_cast<QDoubleSpinBox*>(widget);
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
                         [=] { callback.call(QVariant::fromValue<QDoubleSpinBox*>(spinBox)); });
    } break;
    case Color: {
        auto toolButton = static_cast<QToolButton*>(widget);
        QObject::connect(toolButton, qOverload<bool>(&QToolButton::clicked),
                         [=] { callback.call(QVariant::fromValue<QToolButton*>(toolButton)); });
    } break;
    default:
        break;
    }
}

void PropertiesDelegate::clearWidget(QWidget* widget, PropertiesDelegate::Type type) const
{
    switch (type) {
    case Url:
    case String: {
        auto lineEdit = static_cast<QLineEdit*>(widget);
        lineEdit->deselect();
        lineEdit->clear();
    } break;
    case FontSize:
    case Int: {
        auto spinBox = static_cast<QSpinBox*>(widget);
        spinBox->clear();
    } break;
    case Real: {
        auto spinBox = static_cast<QDoubleSpinBox*>(widget);
        spinBox->clear();
    } break;
    case Enum: {
        auto comboBox = static_cast<QComboBox*>(widget);
        comboBox->clear();
    } break;
    case Bool: {
        auto checkBox = static_cast<QCheckBox*>(widget);
        checkBox->setChecked(false);
    } break;
    default:
        break;
    }
}

int PropertiesDelegate::smartSize(PropertiesDelegate::Type type) const
{
    switch (type) {
    case Url:
        return 3;
    case Enum:
        return 12;
    case Int:
        return 8;
    case String:
        return 7;
    case Bool:
        return 30;
    case Real:
        return 35;
    case Color:
        return 5;
    case FontSize:
        return 4;
    case FontFamily:
    case FontWeight:
    case FontCapitalization:
        return 2;
    default:
        return 0;
    }
}

QWidget* PropertiesDelegate::createWidget(PropertiesDelegate::Type type) const
{
    switch (type) {
    case Url:
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
        comboBox->setFocusPolicy(Qt::StrongFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        TransparentStyle::attach(comboBox);
        return comboBox;
    }

    case Bool: {
        auto checkBox = new QCheckBox;
        checkBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        checkBox->setCursor(Qt::PointingHandCursor);
        checkBox->setFocusPolicy(Qt::StrongFocus);
        checkBox->setSizePolicy(QSizePolicy::Ignored, checkBox->sizePolicy().verticalPolicy());
        checkBox->setMinimumWidth(1);
        checkBox->setFixedWidth(checkBox->style()->pixelMetric(QStyle::PM_IndicatorWidth) + 3);
        checkBox->setStyleSheet("QCheckBox { margin-left: 2px; }");
        return checkBox;
    }

    case Color: {
        auto toolButton = new QToolButton;
        toolButton->setStyleSheet("QToolButton { border: none; background: transparent; }");
        toolButton->setAttribute(Qt::WA_MacShowFocusRect, false);
        toolButton->setIconSize({12, 12});
        toolButton->setCursor(Qt::PointingHandCursor);
        toolButton->setFocusPolicy(Qt::StrongFocus);
        toolButton->setSizePolicy(QSizePolicy::Ignored, toolButton->sizePolicy().verticalPolicy());
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
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
        comboBox->setFocusPolicy(Qt::StrongFocus);
        comboBox->setSizePolicy(QSizePolicy::Ignored, comboBox->sizePolicy().verticalPolicy());
        comboBox->setMinimumWidth(1);
        comboBox->addItems(QFontDatabase().families());
        TransparentStyle::attach(comboBox);
        return comboBox;
    }

    case FontWeight: {
        auto comboBox = new QComboBox;
        comboBox->setAttribute(Qt::WA_MacShowFocusRect, false);
        comboBox->setCursor(Qt::PointingHandCursor);
        comboBox->setFocusPolicy(Qt::StrongFocus);
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
        comboBox->setFocusPolicy(Qt::StrongFocus);
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