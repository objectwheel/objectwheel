#include <inspectorpane.h>
#include <filemanager.h>
#include <saveutils.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <form.h>
#include <projectmanager.h>
#include <wfw.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>

namespace {

bool isProjectStarted = false;
bool isSelectionHandlingBlocked = false;

void fillBackground(QPainter* painter, const QRectF& rect, int row, bool selected, bool verticalLine)
{
    painter->save();

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (selected) {
        painter->fillRect(rect, "#cee7cb");
    } else {
        if (row % 2)
            painter->fillRect(rect, "#edfceb");
        else
            painter->fillRect(rect, Qt::white);
    }

    // Draw top and bottom lines
    painter->setPen("#304A7C42");
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}

void expandAllChildren(QTreeWidget* treeWidget, QTreeWidgetItem* parentItem)
{
    treeWidget->expandItem(parentItem);

    for (int i = 0; i < parentItem->childCount(); ++i)
        expandAllChildren(treeWidget, parentItem->child(i));
}

Control* controlFromItem(const QTreeWidgetItem* item, Form* form)
{
    QList<Control*> allControls;
    allControls.append(form);
    allControls.append(form->childControls());

    for (Control* childControl : allControls) {
        if (childControl->id() == item->text(0))
            return childControl;
    }

    return nullptr;
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

void addChildrenIntoItem(QTreeWidgetItem* parentItem, const QList<Control*>& childItems)
{
    for (const Control* child : childItems) {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, child->id());
        item->setData(0, Qt::UserRole, child->hasErrors());
        item->setData(1, Qt::UserRole, child->hasErrors());
        item->setExpanded(true);

        if (child->gui() && !child->hasErrors())
            item->setText(1, QObject::tr("Yes"));
        else
            item->setText(1, QObject::tr("No"));

        QIcon icon(SaveUtils::toIcon(child->dir()));
        if (icon.isNull())
            icon.addFile(":/images/item.png");

        item->setIcon(0, icon);

        parentItem->addChild(item);
        addChildrenIntoItem(item, child->childControls(false));
    }
}
}
#include <QDebug>
class InspectorListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit InspectorListDelegate(InspectorPane* parent) : QStyledItemDelegate(parent)
      , m_inspectorPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const int iconSize = 15;
        const bool isSelected = option.state & QStyle::State_Selected;
        const QAbstractItemModel* model = index.model();
        const QIcon& icon = model->data(index, Qt::DecorationRole).value<QIcon>();

        QRectF iconRect(0, 0, iconSize, iconSize);
        iconRect.moveCenter(option.rect.center());
        iconRect.moveLeft(option.rect.left() + 5);

        fillBackground(painter, option.rect,
                       calculateVisibleRow(m_inspectorPane->itemFromIndex(index), m_inspectorPane),
                       isSelected, index.column() == 0);

        // Draw icon
        painter->drawPixmap(iconRect, icon.pixmap(wfw(m_inspectorPane), {iconSize, iconSize}),
                            QRectF({}, QSizeF(iconSize * m_inspectorPane->devicePixelRatioF(),
                                              iconSize * m_inspectorPane->devicePixelRatioF())));

        // Draw text
        if (model->data(index, Qt::UserRole).toBool())
            painter->setPen("#cc453b");
        else
            painter->setPen("#254022");

        painter->drawText(option.rect.adjusted(25, 0, 0, 0), index.data(Qt::EditRole).toString(),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), 21);
    }

private:
    InspectorPane* m_inspectorPane;
};

InspectorPane::InspectorPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
{
    setColumnCount(2);
    setIndentation(16);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(new InspectorListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::ExtendedSelection);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->resizeSection(0, 250);
    header()->resizeSection(1, 50);
    headerItem()->setText(0, tr("Controls"));
    headerItem()->setText(1, tr("Ui"));

    setStyleSheet("QTreeView {"
                  "    border: 1px solid #4A7C42;"
                  "} QHeaderView::section {"
                  "    padding-left: 5px;"
                  "    color: white;"
                  "    border: none;"
                  "    border-bottom: 1px solid #4A7C42;"
                  "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                                stop:0 #62A558, stop:1 #599750);"
                  "}");

    connect(this, &InspectorPane::itemSelectionChanged,
            this, &InspectorPane::onItemSelectionChange);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &InspectorPane::onCurrentFormChange);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &InspectorPane::onControlRemove);
    connect(ControlCreationManager::instance(), &ControlCreationManager::controlCreated,
            this, &InspectorPane::onControlCreation);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::formAboutToBeRemoved,
            this, &InspectorPane::onFormRemove);
    connect(this, &InspectorPane::itemDoubleClicked, this, &InspectorPane::onItemDoubleClick);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &InspectorPane::onProjectStart);
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &InspectorPane::onSelectionChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::previewChanged,
            this, &InspectorPane::onControlPreviewChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &InspectorPane::onControlIdChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::parentChanged,
            this, &InspectorPane::onControlParentChange);
}

void InspectorPane::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isSelected = itemFromIndex(index)->isSelected();

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    fillBackground(painter, rect, calculateVisibleRow(itemFromIndex(index), this),
                   isSelected, false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.3);
        pen.setColor("#254022");
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }
    painter->restore();
}

void InspectorPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());

    /* Fill background */
    const qreal bandHeight = topLevelItemCount() ? rowHeight(indexFromItem(topLevelItem(0))) : 21;
    const qreal bandCount = viewport()->height() / bandHeight;

    painter.fillRect(rect(), Qt::white);

    for (int i = 0; i < bandCount; ++i) {
        if (i % 2) {
            painter.fillRect(0, i * bandHeight, viewport()->width(), bandHeight, "#edfceb");
        } else if (topLevelItemCount() == 0) {
            if (i == int(bandCount / 2.0) || i == int(bandCount / 2.0) + 1) {
                painter.setPen("#a6afa5");
                painter.drawText(0, i * bandHeight, viewport()->width(), bandHeight,
                                 Qt::AlignCenter, tr("No items to show"));
            }
        }
    }

    QTreeWidget::paintEvent(e);
}

void InspectorPane::sweep()
{
    qDebug() << "sweep";
    isProjectStarted = false;
    m_formStates.clear();
    clear();
}

void InspectorPane::onProjectStart()
{
    qDebug() << "onProjectStart";
    Q_ASSERT(!isProjectStarted);
    isProjectStarted = true;
    onCurrentFormChange(m_designerScene->currentForm());
}

void InspectorPane::onCurrentFormChange(Form* currentForm)
{
    if (!isProjectStarted)
        return;

    if (!currentForm)
        return;

    qDebug() << "onCurrentFormChange";

    /* Save outgoing form's state */
    if (m_currentForm) {
        FormState state;
        state.verticalScrollBarPosition = verticalScrollBar()->sliderPosition();
        state.horizontalScrollBarPosition = horizontalScrollBar()->sliderPosition();

        for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
            for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
                if (childItem->isSelected())
                    state.selectedIds << childItem->text(0);

                if (!childItem->isExpanded())
                    state.collapsedIds << childItem->text(0);
            }
        }

        m_formStates[m_currentForm] = state;
    }

    /* Clear things */
    m_currentForm = currentForm;
    clear();

    /* Create items for incoming form */
    auto formItem = new QTreeWidgetItem;
    formItem->setText(0, currentForm->id());
    formItem->setData(0, Qt::UserRole, currentForm->hasErrors());
    formItem->setData(1, Qt::UserRole, currentForm->hasErrors());
    formItem->setIcon(0, SaveUtils::isMain(currentForm->dir()) ? QIcon(":/images/mform.png") :
                                                                 QIcon(":/images/form.png"));

    if (currentForm->gui() && !currentForm->hasErrors())
        formItem->setText(1, tr("Yes"));
    else
        formItem->setText(1, tr("No"));

    addChildrenIntoItem(formItem, currentForm->childControls(false));
    addTopLevelItem(formItem);
    sortItems(0, Qt::AscendingOrder);
    expandAll();

    /* Restore incoming form's state */
    const FormState& state = m_formStates.value(currentForm);
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            for (const QString& id : state.selectedIds) {
                if (id == childItem->text(0))
                    childItem->setSelected(true);
            }

            for (const QString& id : state.collapsedIds) {
                if (id == childItem->text(0))
                    childItem->setExpanded(false);
            }
        }
    }

    verticalScrollBar()->setSliderPosition(state.verticalScrollBarPosition);
    horizontalScrollBar()->setSliderPosition(state.horizontalScrollBarPosition);
}

void InspectorPane::onFormRemove(Form* form)
{
    if (!isProjectStarted)
        return;

    qDebug() << "onFormRemove";

    m_formStates.remove(form);
}

void InspectorPane::onControlCreation(Control* control)
{
    if (!isProjectStarted)
        return;

    qDebug() << "onControlCreation";

    const Control* parentControl = control->parentControl();
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (parentControl->id() == childItem->text(0)) {
                addChildrenIntoItem(childItem, QList<Control*>() << control);
                expandAllChildren(this, childItem);
                sortItems(0, Qt::AscendingOrder);
                return;
            }
        }
    }
}

void InspectorPane::onControlRemove(Control* control)
{
    if (!isProjectStarted)
        return;

    qDebug() << "onControlRemove";

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                childItem->parent()->removeChild(childItem);
                delete childItem;
                // No need to following, because the order is preserved after the deletion already.
                // sortItems(0, Qt::AscendingOrder);
                return;
            }
        }
    }
}

void InspectorPane::onControlParentChange(Control* control)
{
    if (!isProjectStarted)
        return;

    qDebug() << "onControlParentChange";

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                for (QTreeWidgetItem* topLevelItem_2 : topLevelItems(this)) {
                    for (QTreeWidgetItem* childItem_2 : allSubChildItems(topLevelItem_2)) {
                        if (control->parentControl()->id() == childItem_2->text(0)) {
                            childItem->parent()->removeChild(childItem);
                            childItem_2->addChild(childItem);
                            sortItems(0, Qt::AscendingOrder);
                            return;
                        }
                    }
                }
                return;
            }
        }
    }
}

void InspectorPane::onControlPreviewChange(Control* control)
{
    if (!isProjectStarted)
        return;

    qDebug() << "onControlPreviewChange";

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                // No need to following, onControlIdChange takes place before the preview operation
                // childItem->setText(0, control->id());
                // Hence no need to this too: sortItems(0, Qt::AscendingOrder);
                childItem->setData(0, Qt::UserRole, control->hasErrors());
                childItem->setData(1, Qt::UserRole, control->hasErrors());

                if (control->gui() && !control->hasErrors())
                    childItem->setText(1, tr("Yes"));
                else
                    childItem->setText(1, tr("No"));

                if (control->form()) {
                    childItem->setIcon(0, SaveUtils::isMain(control->dir()) ?
                                           QIcon(":/images/mform.png") :
                                           QIcon(":/images/form.png"));
                } else {
                    QIcon icon(SaveUtils::toIcon(control->dir()));
                    if (icon.isNull())
                        icon.addFile(":/images/item.png");

                    childItem->setIcon(0, icon);
                }
                return;
            }
        }
    }
}

void InspectorPane::onControlIdChange(Control* control, const QString& previousId)
{
    if (!isProjectStarted)
        return;

    if (previousId.isEmpty())
        return;

    if (control->id() == previousId)
        return;

    if (control->form() && m_designerScene->currentForm() != control)
        return;

    qDebug() << "onControlIdChange";

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (previousId == childItem->text(0)) {
                childItem->setText(0, control->id());
                sortItems(0, Qt::AscendingOrder);
                return;
            }
        }
    }
}

void InspectorPane::onItemDoubleClick(QTreeWidgetItem* item, int)
{
    if (!isProjectStarted)
        return;

    qDebug() << "onItemDoubleClick";

    Control* control = controlFromItem(item, m_designerScene->currentForm());
    if (!control)
        return;

    emit controlDoubleClicked(control);
}

void InspectorPane::onSelectionChange()
{
    if (!isProjectStarted)
        return;

    if (isSelectionHandlingBlocked)
        return;

    qDebug() << "onSelectionChange";

    isSelectionHandlingBlocked = true;

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem))
            childItem->setSelected(false);
    }

    for (const Control* selectedControl : m_designerScene->selectedControls()) {
        for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
            for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
                if (selectedControl->id() == childItem->text(0)) {
                    childItem->setSelected(true);
                    scrollToItem(childItem, PositionAtCenter);
                }
            }
        }
    }

    isSelectionHandlingBlocked = false;
}

void InspectorPane::onItemSelectionChange()
{
    if (!isProjectStarted)
        return;

    if (isSelectionHandlingBlocked)
        return;

    qDebug() << "onItemSelectionChange";

    QList<Control*> selectedControls;
    for (QTreeWidgetItem* item : selectedItems()) {
        Control* control = controlFromItem(item, m_designerScene->currentForm());
        if (!control)
            continue;

        selectedControls.append(control);
    }

    isSelectionHandlingBlocked = true;
    emit controlSelectionChanged(selectedControls);
    isSelectionHandlingBlocked = false;
}

QSize InspectorPane::sizeHint() const
{
    return QSize{340, 240};
}

#include "inspectorpane.moc"
