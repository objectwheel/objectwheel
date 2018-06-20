#include <inspectorpane.h>
#include <css.h>
#include <centralwidget.h>
#include <filemanager.h>
#include <saveutils.h>
#include <savemanager.h>
#include <formspane.h>
#include <controlmonitoringmanager.h>
#include <controlexposingmanager.h>
#include <controlremovingmanager.h>
#include <designerscene.h>
#include <form.h>
#include <projectmanager.h>
#include <dpr.h>
#include <controlmonitoringmanager.h>

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QTimer>
#include <QPointer>

namespace {

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
            painter->fillRect(rect, "#ecfbea");
        else
            painter->fillRect(rect, "#fefffc");
    }

    // Draw top and bottom lines
    painter->setPen("#10000000");
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

    Control* control = nullptr;
    for (Control* childControl : allControls) {
        if (childControl->id() == item->text(0)) {
            control = childControl;
            break;
        }
    }

    return control;
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

QList<QTreeWidgetItem*> allSubChildItems(QTreeWidgetItem* parentItem, bool includeParent = true)
{
    QList<QTreeWidgetItem*> items;

    if (!parentItem)
        return items;

    if (includeParent)
        items.append(parentItem);

    for (int i = 0; i < parentItem->childCount(); i++) {
        items.append(parentItem->child(i));
        items.append(allSubChildItems(parentItem->child(i), false));
    }

    return items;
}

int rowsBelowIndex(const QModelIndex& index) {
    int count = 0;
    const QAbstractItemModel* model = index.model();
    int rowCount = model->rowCount(index);
    count += rowCount;
    for (int r = 0; r < rowCount; ++r)
        count += rowsBelowIndex(model->index(r, 0, index));
    return count;

}

int calculateRow(const QModelIndex& index) {
    int count = 0;
    if (index.isValid()) {
        count = (index.row()) + calculateRow(index.parent());

        const QModelIndex parent = index.parent();
        if (parent.isValid()) {
            ++count;
            for (int r = 0; r < index.row(); ++r)
                count += rowsBelowIndex(parent.child(r, 0));

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

        QIcon icon(child->dir() + separator() + DIR_THIS + separator() + "icon.png");
        if (icon.isNull())
            icon.addFile(":/images/item.png");

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
    InspectorListDelegate(QWidget* parent) : QStyledItemDelegate(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->setRenderHint(QPainter::Antialiasing);

        const int iconSize = 15;
        const bool isSelected = option.state & QStyle::State_Selected;
        const QAbstractItemModel* model = index.model();
        const QIcon& icon = model->data(index, Qt::DecorationRole).value<QIcon>();

        QRectF iconRect(0, 0, iconSize, iconSize);
        iconRect.moveCenter(option.rect.center());
        iconRect.moveLeft(option.rect.left() + 5);

        fillBackground(painter, option.rect, calculateRow(index), isSelected, index.column() == 0);

        // Draw icon
        painter->drawPixmap(iconRect, icon.pixmap(iconSize, iconSize),
                            QRectF({}, QSizeF{iconSize * DPR, iconSize * DPR}));

        // Draw text
        if (model->data(index, Qt::UserRole).toBool())
            painter->setPen("#D02929");
        else
            painter->setPen(qApp->palette().text().color());

        painter->drawText(option.rect.adjusted(25, 0, 0, 0), index.data(Qt::EditRole).toString(),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
    }
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
    verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);

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
    connect(ControlExposingManager::instance(), &ControlExposingManager::controlExposed,
            this, &InspectorPane::onControlAdd);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::formAboutToBeRemoved,
            this, &InspectorPane::onFormRemove);
    connect(this, &InspectorPane::itemDoubleClicked, this, &InspectorPane::onItemDoubleClick);
    connect(ControlMonitoringManager::instance(), &ControlMonitoringManager::idChanged,
            this, &InspectorPane::onControlIdChange);
    // We delay selection change signal, otherwise FormsPane clears the selection (and hence we clear
    // selection on tree view) on the scene before sending a formChanged signal. Thus we can't save
    // selected item states into FormState data for outgoing form.
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &InspectorPane::onSelectionChange, Qt::QueuedConnection);
    // We delay parant change signal, otherwise it is emitted before controlExposed signal for
    // control exposing operation. Thus we get controlAdded slot called twice for control creation.
    connect(ControlMonitoringManager::instance(), &ControlMonitoringManager::parentChanged,
            this, &InspectorPane::onControlParentChange, Qt::QueuedConnection);

    // TODO: Birden çok control'ü reparent edince başka bi kontrole, alt kontrollerin ikonları bozuluyor
    // TODO: Handle icon change
    // TODO: Handle refreshing the list when project loading is done
    // TODO: Handle code changes (ui, non-ui, id change, icon change, error change)
    //    connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(refresh()));
    //    connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(refresh()));
}

void InspectorPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());

    /* Fill background */
    const qreal bandHeight = topLevelItemCount() ? rowHeight(indexFromItem(topLevelItem(0))) : 21;
    const qreal bandCount = viewport()->height() / bandHeight;

    painter.fillRect(rect(), "#fefffc");

    for (int i = 0; i < bandCount; ++i) {
        if (i % 2) {
            painter.fillRect(0, i * bandHeight, viewport()->width(), bandHeight, "#ecfbea");
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

void InspectorPane::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const bool hasChild = itemFromIndex(index)->childCount();
    const bool isSelected = itemFromIndex(index)->isSelected();

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    fillBackground(painter, rect, calculateRow(index), isSelected, false);

    // Draw handle
    if (hasChild) {
        painter->setPen(palette().text().color());
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 1.5, 1.5);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }
}

void InspectorPane::sweep()
{
    m_formStates.clear();
    clear();
}

void InspectorPane::onCurrentFormChange(Form* currentForm)
{
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

    /* Create items for incoming form */
    auto formItem = new QTreeWidgetItem;
    formItem->setText(0, currentForm->id());
    formItem->setText(1, tr("Yes"));
    formItem->setData(0, Qt::UserRole, currentForm->hasErrors());
    formItem->setData(1, Qt::UserRole, currentForm->hasErrors());
    formItem->setIcon(0, SaveUtils::isMain(currentForm->dir()) ? QIcon(":/images/mform.png") :
                                                                 QIcon(":/images/form.png"));

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
    m_formStates.remove(form);
}

void InspectorPane::onControlAdd(Control* control)
{
    const Control* parentControl = control->parentControl();
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (parentControl->id() == childItem->text(0)) {
                addChildrenIntoItem(childItem, QList<Control*>() << control);
                expandAllChildren(this, childItem);
                sortItems(0, Qt::AscendingOrder);
                goto quit;
            }
        }
    }
    quit:;
}

void InspectorPane::onControlRemove(Control* control)
{
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                topLevelItem->removeChild(childItem);
                delete childItem;
                goto quit;
            }
        }
    }
    quit:;
}

void InspectorPane::onControlParentChange(Control* control)
{
    onControlRemove(control);
    onControlAdd(control);
}

void InspectorPane::onControlIdChange(Control* control, const QString& previousId)
{
    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (previousId == childItem->text(0)) {
                childItem->setText(0, control->id());
                goto quit;
            }
        }
    }
    quit:;
}

void InspectorPane::onItemDoubleClick(QTreeWidgetItem* item, int)
{
    Control* control = controlFromItem(item, m_designerScene->currentForm());
    if (!control)
        return;

    emit controlDoubleClicked(control);
}

void InspectorPane::onSelectionChange()
{
    if (isSelectionHandlingBlocked)
        return;

    isSelectionHandlingBlocked = true;

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem))
            childItem->setSelected(false);
    }

    for (const Control* selectedControl : m_designerScene->selectedControls()) {
        for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
            for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
                if (selectedControl->id() == childItem->text(0))
                    childItem->setSelected(true);
            }
        }
    }

    isSelectionHandlingBlocked = false;
}

void InspectorPane::onItemSelectionChange()
{
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
    return QSize{200, 230};
}

#include "inspectorpane.moc"
