#include <inspectorpane.h>
#include <saveutils.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <form.h>
#include <projectmanager.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <toolutils.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>

// FIXME: Make sure you check all currentForm() usages if they are null or not before using them

NavigatorDelegate::NavigatorDelegate(PropertiesTree* propertiesTree) : QStyledItemDelegate(propertiesTree)
  , m_propertiesTree(propertiesTree)
  , m_cache(new NavigatorDelegateCache)
{
}

int NavigatorDelegate::calculateVisibleRow(const QTreeWidgetItem* item) const
{

}

void NavigatorDelegate::paintBackground(QPainter* painter, const QStyleOptionViewItem& option,
                                        int rowNumber, bool hasVerticalLine) const
{
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    const QPalette& pal = option.palette;
    const QRectF& rect = option.rect;

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (isSelected) {
        painter->fillRect(rect, pal.highlight());
    } else {
        if (row % 2)
            painter->fillRect(rect, pal.alternateBase());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    QColor lineColor(pal.dark().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

void NavigatorDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const bool isSelected = option.state & QStyle::State_Selected;
    const QAbstractItemModel* model = index.model();
    const QIcon& icon = model->data(index, Qt::DecorationRole).value<QIcon>();

    QRectF iconRect({}, QSizeF{option.decorationSize});
    iconRect.moveCenter(option.rect.center());
    iconRect.moveLeft(option.rect.left() + 5);

    fillBackground(painter, option,
                   calculateVisibleRow(m_inspectorPane->itemFromIndex(index), m_inspectorPane),
                   index.column() == 0);

    // Draw icon
    Q_ASSERT(UtilityFunctions::window(m_inspectorPane));
    const QPixmap& iconPixmap = icon.pixmap(UtilityFunctions::window(m_inspectorPane),
                                            option.decorationSize,
                                            isSelected ? QIcon::Selected : QIcon::Normal);
    painter->drawPixmap(iconRect, iconPixmap, iconPixmap.rect());

    // Draw text
    if (model->data(index, Qt::UserRole).toBool() && isSelected)
        painter->setPen(option.palette.linkVisited().color().lighter(140));
    else if (model->data(index, Qt::UserRole).toBool() && !isSelected)
        painter->setPen(option.palette.linkVisited().color());
    else if (isSelected)
        painter->setPen(option.palette.highlightedText().color());
    else
        painter->setPen(option.palette.text().color());

    const QRectF& textRect = option.rect.adjusted(option.decorationSize.width() + 10, 0, 0, 0);
    const QString& text = index.data(Qt::DisplayRole).toString();
    painter->drawText(textRect,
                      option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                      QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

    painter->restore();
}

QTreeWidgetItem* NavigatorDelegate::createItem() const
{
    if (QTreeWidgetItem* item = m_cache->pop())
        return item;
    return new QTreeWidgetItem;
}

void NavigatorDelegate::destroyItem(QTreeWidgetItem* item) const
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

void NavigatorDelegate::reserve()
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

NavigatorDelegate::~NavigatorDelegate()
{
    delete m_cache;
}
