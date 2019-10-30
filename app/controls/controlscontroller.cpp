#include <controlscontroller.h>
#include <controlspane.h>
#include <controlstree.h>
#include <controlsdelegate.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <form.h>
#include <projectmanager.h>
#include <lineedit.h>
#include <designersettings.h>
#include <controlssettings.h>

#include <QScrollBar>
#include <QCompleter>

ControlsController::ControlsController(ControlsPane* controlsPane, DesignerScene* designerScene, QObject* parent) : QObject(parent)
  , m_controlsPane(controlsPane)
  , m_designerScene(designerScene)
  , m_isProjectStarted(false)
  , m_isSelectionHandlingBlocked(false)
{
    ControlsTree* tree = m_controlsPane->controlsTree();

    auto completer = new QCompleter(this);
    completer->setModel(&m_searchCompleterModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    m_controlsPane->searchEdit()->setCompleter(completer);

    connect(m_controlsPane->searchEdit(), &LineEdit::returnPressed,
            this, &ControlsController::onSearchEditReturnPress);
    connect(tree, &ControlsTree::itemDoubleClicked,
            this, &ControlsController::onItemDoubleClick);
    connect(tree, &ControlsTree::itemSelectionChanged,
            this, &ControlsController::onItemSelectionChange);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &ControlsController::onCurrentFormChange);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &ControlsController::onControlRemove);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &ControlsController::onProjectStart);
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &ControlsController::onSceneSelectionChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
            tree, qOverload<>(&ControlsTree::update));
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &ControlsController::onControlIdChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::indexChanged,
            this, &ControlsController::onControlIndexChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::parentChanged,
            this, &ControlsController::onControlParentChange);
}

Control* ControlsController::controlFromItem(const QTreeWidgetItem* item) const
{
    return item->data(0, ControlsDelegate::ControlRole).value<QPointer<Control>>().data();
}

QTreeWidgetItem* ControlsController::itemFromControl(const Control* control) const
{
    ControlsTree* tree = m_controlsPane->controlsTree();
    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (controlFromItem(item) == control)
            return item;
    }
    return nullptr;
}

void ControlsController::discharge()
{
    m_isProjectStarted = false;
    m_formStates.clear();
    m_controlsPane->searchEdit()->clear();
    clear();
}

void ControlsController::clear()
{
    m_isSelectionHandlingBlocked = true;
    ControlsTree* tree = m_controlsPane->controlsTree();
    EVERYTHING(QTreeWidgetItem* item, tree)
            tree->delegate()->destroyItem(item);
    m_searchCompleterModel.setStringList({});
    m_isSelectionHandlingBlocked = false;
}

void ControlsController::onProjectStart()
{
    Q_ASSERT(!m_isProjectStarted);
    Q_ASSERT(m_designerScene->currentForm());
    m_isProjectStarted = true;
    onCurrentFormChange(m_designerScene->currentForm());
}

void ControlsController::onSearchEditReturnPress()
{
    if (!m_isProjectStarted)
        return;

    ControlsTree* tree = m_controlsPane->controlsTree();
    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (const Control* control = controlFromItem(item)) {
            if (QString::compare(control->id(), m_controlsPane->searchEdit()->text(), Qt::CaseInsensitive) == 0) {
                m_isSelectionHandlingBlocked = true;
                tree->clearSelection();
                m_isSelectionHandlingBlocked = false;
                item->setSelected(true);
                tree->scrollToItem(item);
                return;
            }
        }
    }
}

void ControlsController::onItemDoubleClick(QTreeWidgetItem* item, int)
{
    if (!m_isProjectStarted)
        return;

    ControlsTree* tree = m_controlsPane->controlsTree();
    m_isSelectionHandlingBlocked = true;
    tree->clearSelection();
    m_isSelectionHandlingBlocked = false;
    item->setSelected(true);
    tree->scrollToItem(item);

    const ControlsSettings* settings = DesignerSettings::controlsSettings();
    if (QGraphicsItem* mouseGrabber = m_designerScene->mouseGrabberItem())
        mouseGrabber->ungrabMouse();
    if (settings->itemDoubleClickAction == 0)
        emit editAnchorsActionTriggered();
    else if (settings->itemDoubleClickAction == 1)
        emit viewSourceCodeActionTriggered();
    else
        emit goToSlotActionTriggered();
}

void ControlsController::onItemSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    const QList<QTreeWidgetItem*>& selectedItems = m_controlsPane->controlsTree()->selectedItems();

    QList<Control*> selectedControls;
    for (QTreeWidgetItem* item : selectedItems) {
        if (Control* control = controlFromItem(item))
            selectedControls.append(control);
    }

    m_isSelectionHandlingBlocked = true;
    emit controlSelectionChanged(selectedControls);
    m_isSelectionHandlingBlocked = false;
}

void ControlsController::onSceneSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    ControlsTree* tree = m_controlsPane->controlsTree();
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();

    m_isSelectionHandlingBlocked = true;

    tree->clearSelection();
    QTreeWidgetItem* firstSelectedItem = nullptr;
    for (const Control* selectedControl : selectedControls) {
        if (QTreeWidgetItem* item = itemFromControl(selectedControl)) {
            item->setSelected(true);
            if (firstSelectedItem == 0)
                firstSelectedItem = item;
        }
    }

    m_isSelectionHandlingBlocked = false;

    tree->scrollToItem(firstSelectedItem);
}

void ControlsController::onControlCreation(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->scene() == 0)
        return;

    ControlsTree* tree = m_controlsPane->controlsTree();

    if (const Control* parentControl = control->parentControl()) {
        EVERYTHING(QTreeWidgetItem* item, tree) {
            if (const Control* ctrl = controlFromItem(item)) {
                if (ctrl == parentControl) {
                    addControls(item, QList<Control*>{control});
                    expandRecursive(item);
                    tree->sortItems(0, Qt::AscendingOrder);
                    return;
                }
            }
        }
    }
}

void ControlsController::onControlRemove(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->type() == Form::Type) {
        m_formStates.remove(static_cast<Form*>(control));

        // Form items will be cleared right after this slot,
        // when onCurrentFormChange slot is called.
    } else {
        if (m_currentForm == 0)
            return;

        if (!m_currentForm->isAncestorOf(control))
            return;

        ControlsTree* tree = m_controlsPane->controlsTree();

        EVERYTHING(QTreeWidgetItem* item, tree) {
            if (const Control* ctrl = controlFromItem(item)) {
                if (ctrl == control) {
                    const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(item);
                    for (QTreeWidgetItem* child : childs) {
                        if (const Control* ctrl2 = controlFromItem(child)) {
                            removeCompleterEntry(ctrl2->id());
                            m_isSelectionHandlingBlocked = true;
                            tree->delegate()->destroyItem(child);
                            m_isSelectionHandlingBlocked = false;
                        }
                    }
                    // No need to sort, because the order is
                    // already preserved after the deletion
                    return;
                }
            }
        }
    }
}

void ControlsController::onCurrentFormChange(Form* currentForm)
{
    if (!m_isProjectStarted)
        return;

    ControlsTree* tree = m_controlsPane->controlsTree();

    /* Save outgoing form's state */
    if (m_currentForm) {
        FormState state;
        EVERYTHING(QTreeWidgetItem* item, tree) {
            if (Control* control = controlFromItem(item)) {
                if (!item->isExpanded())
                    state.collapsedControls.append(control);
            }
        }
        state.verticalScrollBarPosition = tree->verticalScrollBar()->sliderPosition();
        state.horizontalScrollBarPosition = tree->horizontalScrollBar()->sliderPosition();
        m_formStates.insert(m_currentForm, state);
    }

    /* Clear things */
    m_currentForm = currentForm;
    clear();

    if (m_currentForm == 0)
        return;

    /* Create items for incoming form */
    auto formItem = tree->delegate()->createItem(m_currentForm);
    addCompleterEntry(m_currentForm->id());
    addControls(formItem, m_currentForm->childControls(false));

    tree->addTopLevelItem(formItem);
    tree->sortItems(0, Qt::AscendingOrder);
    tree->expandAll();

    /* Restore incoming form's state */
    QTreeWidgetItem* firstSelectedItem = nullptr;
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    const FormState& state = m_formStates.value(m_currentForm);

    m_isSelectionHandlingBlocked = true;

    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (Control* control = controlFromItem(item)) {
            if (selectedControls.contains(control)) {
                item->setSelected(true);
                if (firstSelectedItem == 0)
                    firstSelectedItem = item;
            }
            if (state.collapsedControls.contains(control))
                item->setExpanded(false);
        }
    }

    m_isSelectionHandlingBlocked = false;

    tree->scrollToItem(firstSelectedItem);
    tree->verticalScrollBar()->setSliderPosition(state.verticalScrollBarPosition);
    tree->horizontalScrollBar()->setSliderPosition(state.horizontalScrollBarPosition);
}

void ControlsController::onControlParentChange(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (!control->parentControl())
        return;

    ControlsTree* tree = m_controlsPane->controlsTree();

    // Check if already exists, if not --reparented from
    // another form or a tool dropped-- add it to the list
    if (itemFromControl(control) == 0)
        return onControlCreation(control);

    if (QTreeWidgetItem* parentItem = itemFromControl(control->parentControl())) {
        if (QTreeWidgetItem* childItem = itemFromControl(control)) {
            bool isCollapsed = !childItem->isExpanded();
            childItem->parent()->removeChild(childItem);
            parentItem->addChild(childItem);
            childItem->setExpanded(!isCollapsed);
            tree->sortItems(0, Qt::AscendingOrder);
        }
    }
}

void ControlsController::onControlIndexChange(Control* control) const
{
    if (!m_isProjectStarted)
        return;

    if (control->type() == Form::Type)
        return;

    if (m_currentForm == 0)
        return;

    if (!m_currentForm->isAncestorOf(control))
        return;

    m_controlsPane->controlsTree()->sortItems(0, Qt::AscendingOrder);
}

void ControlsController::onControlIdChange(Control* control, const QString& previousId)
{
    if (!m_isProjectStarted)
        return;

    if (previousId.isEmpty())
        return;

    if (control->id() == previousId)
        return;

    if (control->type() == Form::Type && control != m_currentForm)
        return;

    if (control != m_currentForm && !m_currentForm->isAncestorOf(control))
        return;

    removeCompleterEntry(previousId);
    addCompleterEntry(control->id());
    m_controlsPane->controlsTree()->update();
}

void ControlsController::addCompleterEntry(const QString& entry)
{
    QStringList list(m_searchCompleterModel.stringList());
    list.append(entry);
    m_searchCompleterModel.setStringList(list);
}

void ControlsController::removeCompleterEntry(const QString& entry)
{
    QStringList list(m_searchCompleterModel.stringList());
    list.removeOne(entry);
    m_searchCompleterModel.setStringList(list);
}

void ControlsController::expandRecursive(const QTreeWidgetItem* parentItem)
{
    m_controlsPane->controlsTree()->expandItem(parentItem);
    for (int i = 0; i < parentItem->childCount(); ++i)
        expandRecursive(parentItem->child(i));
}

void ControlsController::addControls(QTreeWidgetItem* parentItem, const QList<Control*>& controls)
{
    ControlsTree* tree = m_controlsPane->controlsTree();
    for (Control* control : controls) {
        QTreeWidgetItem* item = tree->delegate()->createItem(control);
        parentItem->addChild(item);
        addCompleterEntry(control->id());
        addControls(item, control->childControls(false));
    }
}
