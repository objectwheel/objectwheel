#include <propertiespane.h>
#include <focuslesslineedit.h>
#include <css.h>
#include <filemanager.h>
#include <saveutils.h>
#include <controlpropertymanager.h>
//#include <controlcreationmanager.h>
//#include <controlremovingmanager.h>
#include <designerscene.h>
//#include <form.h>
//#include <projectmanager.h>
#include <dpr.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>

namespace {

void fillBackground(QPainter* painter, const QRectF& rect, int row, bool selected, bool classRow,
                    bool verticalLine)
{
    painter->save();

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (classRow)
        painter->fillRect(rect, "#9D7650");
    else if (selected)
        painter->fillRect(rect, "#ebd5c0");
    else if (row % 2)
        painter->fillRect(rect, "#faf1e8");
    else
        painter->fillRect(rect, "#fffefc");

    // Draw top and bottom lines
    painter->setPen("#25000000");
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

QList<QTreeWidgetItem*> topLevelItems(const QTreeWidget* treeWidget)
{
    QList<QTreeWidgetItem*> items;

    if (!treeWidget)
        return items;

    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        items.append(treeWidget->topLevelItem(i));

    return items;
}

QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true,
                                         bool includeCollapsed = true)
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if (!includeCollapsed && !parentItem->isExpanded()) {
        if (includeParent)
            items.append(parentItem);

        return items;
    }

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false, includeCollapsed));
    }

    return items;
}

int calculateVisibleRow(const QTreeWidgetItem* item, const QTreeWidget* treeWidget)
{
    int count = 0;
    for (QTreeWidgetItem* topLevelItem : topLevelItems(treeWidget)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem, true, false)) {
            if (childItem == item)
                return count;

            ++count;
        }
    }

    return count;
}

QWidget* createIdWidget(const QString& text, Control* selectedControl)
{
    auto lineEdit = new QLineEdit;
    lineEdit->setValidator(new QRegExpValidator(QRegExp("[a-z_][a-zA-Z0-9_]+"), lineEdit));
    lineEdit->setStyleSheet("border:none; background: transparent;");
    lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    lineEdit->setText(text);

    QObject::connect(lineEdit, &QLineEdit::editingFinished, [=]
    {
        if (selectedControl->id() != lineEdit->text())
            ControlPropertyManager::setId(selectedControl, lineEdit->text());
    });

    return lineEdit;
}
}

class PropertiesListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit PropertiesListDelegate(PropertiesPane* parent) : QStyledItemDelegate(parent)
      , m_propertiesPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        painter->setRenderHint(QPainter::Antialiasing);

        const QAbstractItemModel* model = index.model();
        const bool isSelected = option.state & QStyle::State_Selected;
        const bool isClassRow = !model->parent(index).isValid() && index.row() > 2;

        fillBackground(painter, option.rect,
                       calculateVisibleRow(m_propertiesPane->itemFromIndex(index), m_propertiesPane),
                       isSelected, isClassRow, index.column() == 0 && !isClassRow);

        // Draw data
        if (index.column() == 0) {
            if (isClassRow)
                painter->setPen(Qt::white);
            else
                painter->setPen(Qt::black);

            painter->drawText(option.rect.adjusted(5, 0, 0, 0), index.data(Qt::EditRole).toString(),
                              QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        }
    }

private:
    PropertiesPane* m_propertiesPane;
};

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_searchEdit(new FocuslessLineEdit(this))
{
    setColumnCount(2);
    setIndentation(16);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(new PropertiesListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->resizeSection(0, 180);
    header()->resizeSection(1, 120);
    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(0, tr("Property"));
    verticalScrollBar()->setStyleSheet(CSS_SCROLLBAR);
    horizontalScrollBar()->setStyleSheet(CSS_SCROLLBAR_H);
    setStyleSheet("QTreeView {"
                  "    border: 1px solid #8c6a48;"
                  "    outline: 0;"
                  "} QHeaderView::section {"
                  "    padding-left: 5px;"
                  "    color: white;"
                  "    border: none;"
                  "    border-bottom: 1px solid #8c6a48;"
                  "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                                stop:0 #AB8157, stop:1 #9C7650);"
                  "}");

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    //    connect(m_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    /* Prepare Properties Widget */
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &PropertiesPane::onSelectionChange);
    //    connect(m_designerScene, SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(zValueChanged(Control*)), SLOT(handleSelectionChange()));
}

void PropertiesPane::sweep()
{
    // TODO
}

void PropertiesPane::onSelectionChange()
{
    const int verticalScrollBarPosition = verticalScrollBar()->sliderPosition();
    const int horizontalScrollBarPosition = horizontalScrollBar()->sliderPosition();

    clear();

    if (m_designerScene->selectedControls().size() != 1)
        return;

    Control* selectedControl = m_designerScene->selectedControls().first();
    setDisabled(selectedControl->hasErrors());

    const QList<PropertyNode>& properties = selectedControl->properties();
    if (properties.isEmpty())
        return;

    QTreeWidgetItem* typeItem = new QTreeWidgetItem;
    typeItem->setText(0, tr("Type"));
    typeItem->setText(1, properties.first().cleanClassName);
    addTopLevelItem(typeItem);

    QTreeWidgetItem* uidItem = new QTreeWidgetItem;
    uidItem->setText(0, "uid");
    uidItem->setText(1, selectedControl->uid());
    addTopLevelItem(uidItem);

    QTreeWidgetItem* idItem = new QTreeWidgetItem;
    idItem->setText(0, "id");
    addTopLevelItem(idItem);
    setItemWidget(idItem, 1, createIdWidget(selectedControl->id(), selectedControl));



    for (const auto& propertyNode : properties) {
        auto map = propertyNode.properties;
        auto enums = propertyNode.enums;

        if (map.isEmpty() && enums.isEmpty())
            continue;

        auto item = new QTreeWidgetItem;
        item->setText(0, propertyNode.cleanClassName);

        for (const auto& propertyName : map.keys()) {
            switch (map.value(propertyName).type()) {
            case QVariant::Font: {
                const QFont& font = map.value(propertyName).value<QFont>();
//                addFontChild(item, propertyName, font);
                break;
            }

            case QVariant::Color: {
                const QColor& value = map.value(propertyName).value<QColor>();
                const QString& colorName = value.name(QColor::HexArgb);
//                addChild(item, NodeType::Color, propertyName, value, colorName);
                break;
            }

            case QVariant::Bool: {
                const bool value = map.value(propertyName).value<bool>();
//                addChild(item, NodeType::Bool, propertyName, value, QVariant());
                break;
            }

            case QVariant::String: {
                const QString& value = map.value(propertyName).value<QString>();
//                addChild(item, NodeType::String, propertyName, value, value);
                break;
            }

            case QVariant::Url: {
                const QUrl& value = map.value(propertyName).value<QUrl>();
                const QString& relativePath = m_designerScene->selectedControls().first()->dir();
                QString displayText = value.toDisplayString();

                if (value.isLocalFile())
                    displayText = value.toLocalFile().remove(SaveUtils::toThisDir(relativePath) + separator());

//                addChild(item, NodeType::Url, propertyName, value, displayText);
                break;
            }

            case QVariant::Double: {
                if (propertyName == "x" || propertyName == "y" ||
                        propertyName == "width" || propertyName == "height") {
//                    if (propertyName == "x") //To make it called only once
//                        addGeometryChild(item, "geometry", selectedControl->rect(), true);
                } else {
                    if (propertyName == "z")
                        map[propertyName] = selectedControl->zValue();
                    const double value = map.value(propertyName).value<double>();
//                    addChild(item, NodeType::Double, propertyName, value, value);
                }
                break;
            }

            case QVariant::Int: {
                if (propertyName == "x" || propertyName == "y" ||
                        propertyName == "width" || propertyName == "height") {
//                    if (propertyName == "x")
//                        addGeometryChild(item, "geometry", selectedControl->rect(), false);
                } else {
                    const int value = map.value(propertyName).value<int>();
//                    addChild(item, NodeType::Int, propertyName, value, value);
                }
                break;
            }

            default: {
                continue;
                // QTreeWidgetItem* iitem = new QTreeWidgetItem;
                // iitem->setText(0, propertyName);
                // iitem->setText(1, map.value(propertyName).typeName());
                // item->addChild(iitem);
                // break;
            }
            }
        }

        for (auto e : enums) {
            auto item1 = new QTreeWidgetItem;
//            item1->setFlags(item1->flags() | Qt::ItemIsEditable);
            item1->setText(0, e.name);
//            item1->setData(1, Qt::EditRole, e.value);
//            item1->setData(1, NodeRole::Type, NodeType::EnumType);
//            item1->setData(1, NodeRole::Data, QVariant::fromValue(e));
            item->addChild(item1);
        }

        addTopLevelItem(item);
        expandItem(item);
    }

//    filterList(m_searchEdit->text());

    verticalScrollBar()->setSliderPosition(verticalScrollBarPosition);
    horizontalScrollBar()->setSliderPosition(horizontalScrollBarPosition);
}

void PropertiesPane::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const QAbstractItemModel* model = index.model();
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isClassRow = !model->parent(index).isValid() && index.row() > 2;
    const bool isSelected = itemFromIndex(index)->isSelected();

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    fillBackground(painter, rect, calculateVisibleRow(itemFromIndex(index), this),
                   isSelected, isClassRow, false);

    // Draw handle
    if (hasChild) {
        painter->setPen(isClassRow ? Qt::white : Qt::black);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }
}

void PropertiesPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());

    /* Fill background */
    const qreal bandHeight = topLevelItemCount() ? rowHeight(indexFromItem(topLevelItem(0))) : 21;
    const qreal bandCount = viewport()->height() / bandHeight;
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();

    painter.fillRect(rect(), "#fffefc");

    for (int i = 0; i < bandCount; ++i) {
        if (i % 2) {
            painter.fillRect(0, i * bandHeight, viewport()->width(), bandHeight, "#faf1e8");
        } else if (topLevelItemCount() == 0) {
            if (i == int(bandCount / 2.0) || i == int(bandCount / 2.0) + 1) {
                QString message;
                if (selectedControls.size() == 0)
                    message = tr("No controls selected");
                else if (selectedControls.size() == 1)
                    message = tr("Control has errors");
                else
                    message = tr("Multiple controls selected");

                painter.setPen(selectedControls.size() == 1 ? "#d98083" : "#b5aea7");
                painter.drawText(0, i * bandHeight, viewport()->width(), bandHeight,
                                 Qt::AlignCenter, message);
            }
        }
    }

    QTreeWidget::paintEvent(e);
}

void PropertiesPane::updateGeometries()
{
    QTreeWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    QRect vg = viewport()->geometry();
    QRect geometryRect(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    setViewportMargins(vm);
    m_searchEdit->setGeometry(geometryRect);
}

QSize PropertiesPane::sizeHint() const
{
    return QSize{340, 700};
}

#include "propertiespane.moc"
