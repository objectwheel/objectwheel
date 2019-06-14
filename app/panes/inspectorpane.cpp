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

namespace {

const int ROW_HEIGHT = 21;
bool isProjectStarted = false;
bool isSelectionHandlingBlocked = false;

void initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#62A558");
    palette.setColor(QPalette::Dark, "#599750");
    palette.setColor(QPalette::AlternateBase, "#e8f7e6");
    widget->setPalette(palette);
}

void fillBackground(QPainter* painter, const QStyleOptionViewItem& option, int row, bool verticalLine)
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
    QTreeWidget* treeWidget = parentItem->treeWidget();
    Q_ASSERT(treeWidget);

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

        QIcon icon;
        icon.addPixmap(PaintUtils::renderOverlaidPixmap(ToolUtils::toolIcon(child->dir(), treeWidget->devicePixelRatioF()),
                                                        treeWidget->palette().text().color(),
                                                        treeWidget), QIcon::Normal);
        icon.addPixmap(PaintUtils::renderOverlaidPixmap(ToolUtils::toolIcon(child->dir(), treeWidget->devicePixelRatioF()),
                                                        treeWidget->palette().highlightedText().color(),
                                                        treeWidget), QIcon::Selected);
        item->setIcon(0, icon);
        parentItem->addChild(item);
        addChildrenIntoItem(item, child->childControls(false));
    }
}
}

class InspectorListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit InspectorListDelegate(InspectorPane* parent) : QStyledItemDelegate(parent)
      , m_inspectorPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
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

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), ROW_HEIGHT);
    }

private:
    InspectorPane* m_inspectorPane;
};

InspectorPane::InspectorPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
{
    initPalette(this);

    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->resizeSection(0, 220); // Don't resize the last (stretched) column

    headerItem()->setText(0, tr("Controls"));
    headerItem()->setText(1, tr("Ui"));

    setColumnCount(2);
    setIndentation(16);
    setIconSize({15, 15});
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(new InspectorListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(
                QString {
                    "QTreeView {"
                    "    border: 1px solid %1;"
                    "} QHeaderView::section {"
                    "    color: %4;"
                    "    padding-left: 5px;"
                    "    padding-top: 3px;"
                    "    padding-bottom: 3px;"
                    "    border-style: solid;"
                    "    border-left-width: 0px;"
                    "    border-top-width: 0px;"
                    "    border-bottom-color: %1;"
                    "    border-bottom-width: 1px;"
                    "    border-right-color: %1; "
                    "    border-right-width: 1px;"
                    "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                    "                                stop:0 %2, stop:1 %3);"
                    "}"
                    "QHeaderView::section:last{"
                    "    border-left-width: 0px;"
                    "    border-right-width: 0px;"
                    "}"
                }
                .arg(palette().dark().color().darker(120).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name())
                );

    // WARNING: Beware, ControlPropertyManager signals are emitted everytime a setProperty called
    // no matter what. I think we should consider reviewing related slots against possible miscalls
    connect(this, &InspectorPane::itemSelectionChanged,
            this, &InspectorPane::onItemSelectionChange);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &InspectorPane::onCurrentFormChange);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &InspectorPane::onControlRemove);
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

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;
    if (isSelected)
        option.state |= QStyle::State_Selected;
    else if (option.state & QStyle::State_Selected)
        option.state ^= QStyle::State_Selected;

    fillBackground(painter, option, calculateVisibleRow(itemFromIndex(index), this), false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.2);
        pen.setColor(isSelected ? palette().highlightedText().color() : palette().text().color());
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
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().dark().color());
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    for (int i = 0; i < viewport()->height() / qreal(ROW_HEIGHT); ++i) {
        QRectF rect(0, i * ROW_HEIGHT, viewport()->width(), ROW_HEIGHT);
        QPainterPath path;
        path.addRect(rect);
        painter.setClipPath(path);

        // Fill background
        if (i % 2)
            painter.fillRect(rect, palette().alternateBase());

        // Draw vertical line
        QRectF cell(rect);
        cell.setSize(QSizeF(header()->sectionSize(0), rect.height()));
        painter.drawLine(cell.topRight() + QPointF(-0.5, 0.5),
                         cell.bottomRight() + QPointF(-0.5, -0.5));

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
    }

    QTreeWidget::paintEvent(e);
}

void InspectorPane::discharge()
{
    isProjectStarted = false;
    m_formStates.clear();
    clear();
}

void InspectorPane::onProjectStart()
{
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
    m_designerScene->clearSelection();

    QIcon formIcon;
    formIcon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/form.png",
                                                        palette().text().color(),
                                                        this), QIcon::Normal);
    formIcon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/form.png",
                                                        palette().highlightedText().color(),
                                                        this), QIcon::Selected);

    /* Create items for incoming form */
    auto formItem = new QTreeWidgetItem;
    formItem->setText(0, currentForm->id());
    formItem->setData(0, Qt::UserRole, currentForm->hasErrors());
    formItem->setData(1, Qt::UserRole, currentForm->hasErrors());
    formItem->setIcon(0, formIcon);

    if (currentForm->gui() && !currentForm->hasErrors())
        formItem->setText(1, tr("Yes"));
    else
        formItem->setText(1, tr("No"));

    addTopLevelItem(formItem);
    addChildrenIntoItem(formItem, currentForm->childControls(false));
    sortItems(0, Qt::AscendingOrder);
    expandAll();

    /* Restore incoming form's state */
    bool scrolled = false;
    const FormState& state = m_formStates.value(currentForm);
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            for (const QString& id : state.selectedIds) {
                if (id == childItem->text(0)) {
                    childItem->setSelected(true);
                    if (!scrolled) {
                        scrollToItem(childItem, PositionAtCenter);
                        scrolled = true;
                    }
                }
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

    m_formStates.remove(form);
}

void InspectorPane::onControlCreation(Control* control)
{
    if (!isProjectStarted)
        return;

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

    // Check if already exists, if not --reparented from another form-- add it to the list
    bool found = false;
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0))
                found = true;
        }
    }

    if (!found)
        return onControlCreation(control);

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                for (QTreeWidgetItem* topLevelItem_2 : topLevelItems(this)) {
                    for (QTreeWidgetItem* childItem_2 : allSubChildItems(topLevelItem_2)) {
                        if (control->parentControl()->id() == childItem_2->text(0)) {
                            bool isExpanded = childItem->isExpanded();
                            childItem->parent()->removeChild(childItem);
                            childItem_2->addChild(childItem);
                            childItem->setExpanded(isExpanded);
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

void InspectorPane::onControlPreviewChange(Control* control, bool codeChanged)
{
    if (!isProjectStarted)
        return;

    if (!codeChanged)
        return;

    if (control->form() && m_designerScene->currentForm() != control)
        return;

    if (m_designerScene->currentForm() != control
            && !m_designerScene->currentForm()->isAncestorOf(control)) {
        return;
    }

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
                    QIcon formIcon;
                    formIcon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/form.png",
                                                                        palette().text().color(),
                                                                        this), QIcon::Normal);
                    formIcon.addPixmap(PaintUtils::renderOverlaidPixmap(":/images/form.png",
                                                                        palette().highlightedText().color(),
                                                                        this), QIcon::Selected);
                    childItem->setIcon(0, formIcon);
                } else {
                    QIcon icon;
                    icon.addPixmap(PaintUtils::renderOverlaidPixmap(ToolUtils::toolIcon(control->dir(), devicePixelRatioF()),
                                                                    palette().text().color(),
                                                                    this), QIcon::Normal);
                    icon.addPixmap(PaintUtils::renderOverlaidPixmap(ToolUtils::toolIcon(control->dir(), devicePixelRatioF()),
                                                                    palette().highlightedText().color(),
                                                                    this), QIcon::Selected);
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

    if (m_designerScene->currentForm() != control
            && !m_designerScene->currentForm()->isAncestorOf(control)) {
        return;
    }

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (previousId == childItem->text(0)) {
                childItem->setText(0, control->id());
                sortItems(0, Qt::AscendingOrder);
                goto phase2;
            }
        }
    }

phase2:

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (childItem->isSelected()) {
                scrollToItem(childItem, QAbstractItemView::PositionAtCenter);
                return;
            }
        }
    }
}

void InspectorPane::onItemDoubleClick(QTreeWidgetItem* item, int)
{
    if (!isProjectStarted)
        return;

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

    isSelectionHandlingBlocked = true;

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem))
            childItem->setSelected(false);
    }

    bool scrolled = false;
    for (const Control* selectedControl : m_designerScene->selectedControls()) {
        for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
            for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
                if (selectedControl->id() == childItem->text(0)) {
                    childItem->setSelected(true);
                    if (!scrolled) {
                        scrollToItem(childItem, PositionAtCenter);
                        scrolled = true;
                    }
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
    return QSize{310, 220};
}

#include "inspectorpane.moc"
