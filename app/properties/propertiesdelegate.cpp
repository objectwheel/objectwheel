#include <propertiesdelegate.h>
#include <propertiestree.h>
#include <propertiescache.h>
#include <QPainter>
#include <QLineEdit>
#include <QCheckBox>

static void setInitialValue(QWidget* widget, PropertiesCache::Type type, const QVariant& value)
{
    const char* propertyName = 0;
    switch (type) {
    case PropertiesCache::String:
        propertyName = "text";
        break;
    case PropertiesCache::Enum:
        propertyName = "currentText";
        break;
    case PropertiesCache::Bool:
        propertyName = "checked";
        break;
    case PropertiesCache::Color:
        propertyName = "color";
        break;
    case PropertiesCache::Int:
        propertyName = "value";
        break;
    case PropertiesCache::Real:
        propertyName = "value";
        break;
    case PropertiesCache::FontSize:
        propertyName = "value";
        break;
    case PropertiesCache::FontFamily:
        propertyName = "currentText";
        break;
    case PropertiesCache::FontWeight:
        propertyName = "currentText";
        break;
    case PropertiesCache::FontCapitalization:
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
{}

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
        if (index.column() == 0 && index.data(PropertiesTree::ModificationRole).toBool()) {
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

    auto type = index.data(PropertiesTree::TypeRole).value<PropertiesCache::Type>();
    if (type == PropertiesCache::Invalid)
        return QStyledItemDelegate::createEditor(parent, option, index);

    QWidget* widget = m_propertiesTree->cache()->pop(type);
    widget->setParent(parent);
    widget->setVisible(true);
    setInitialValue(widget, type, index.data(PropertiesTree::InitialValueRole));
    addConnection(widget, type, index.data(PropertiesTree::PropertyNameRole).toString());
    return widget;
}

void PropertiesDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() == 0)
        return editor->deleteLater();

    auto type = index.data(PropertiesTree::TypeRole).value<PropertiesCache::Type>();
    if (type == PropertiesCache::Invalid)
        return QStyledItemDelegate::destroyEditor(editor, index);

    clearConnection(editor);
    m_propertiesTree->cache()->push(type, editor);
}

void PropertiesDelegate::addConnection(QWidget* widget, int type, const QString& propertyName) const
{
    switch (type) {
    case PropertiesCache::String: {
        auto lineEdit = static_cast<QLineEdit*>(widget);
        connect(lineEdit, &QLineEdit::editingFinished,
                [=] { emit propertyEdited(type, propertyName, lineEdit->text()); });
    } break;
//    case PropertiesCache::Enum:
//        propertyName = "currentText";
//        break;
    case PropertiesCache::Bool: {
        auto checkBox = static_cast<QCheckBox*>(widget);
        connect(checkBox, qOverload<bool>(&QCheckBox::clicked),
                [=] { emit propertyEdited(type, propertyName, checkBox->isChecked()); });
    } break;
//    case PropertiesCache::Color:
//        propertyName = "color";
//        break;
//    case PropertiesCache::Int:
//        propertyName = "value";
//        break;
//    case PropertiesCache::Real:
//        propertyName = "value";
//        break;
//    case PropertiesCache::FontSize:
//        propertyName = "value";
//        break;
//    case PropertiesCache::FontFamily:
//        propertyName = "currentText";
//        break;
//    case PropertiesCache::FontWeight:
//        propertyName = "currentText";
//        break;
//    case PropertiesCache::FontCapitalization:
//        propertyName = "currentText";
//        break;
    default:
        break;
    }
}

void PropertiesDelegate::clearConnection(QWidget* widget) const
{
    disconnect(widget, 0, 0, 0);
}
