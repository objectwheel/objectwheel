#include <navigatorcontroller.h>
#include <navigatorpane.h>
#include <navigatortree.h>
#include <navigatordelegate.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <form.h>
#include <projectmanager.h>
#include <lineedit.h>
#include <designersettings.h>
#include <navigatorsettings.h>

#include <QScrollBar>
#include <QCompleter>

NavigatorController::NavigatorController(NavigatorPane* navigatorPane, DesignerScene* designerScene, QObject* parent) : QObject(parent)
  , m_navigatorPane(navigatorPane)
  , m_designerScene(designerScene)
  , m_isSelectionHandlingBlocked(false)
  , m_isProjectStarted(false)
{
    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    auto completer = new QCompleter(this);
    completer->setModel(&m_searchCompleterModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    m_navigatorPane->searchEdit()->setCompleter(completer);

    connect(m_navigatorPane->searchEdit(), &LineEdit::returnPressed,
            this, &NavigatorController::onSearchEditReturnPress);
    connect(tree, &NavigatorTree::itemDoubleClicked,
            this, &NavigatorController::onItemDoubleClick);
    connect(tree, &NavigatorTree::itemSelectionChanged,
            this, &NavigatorController::onItemSelectionChange);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &NavigatorController::onCurrentFormChange);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &NavigatorController::onControlRemove);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &NavigatorController::onFormRemove);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &NavigatorController::onProjectStart);
    connect(m_designerScene, &DesignerScene::selectionChanged,
            this, &NavigatorController::onSceneSelectionChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
            tree, qOverload<>(&NavigatorTree::update));
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &NavigatorController::onControlIdChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::indexChanged,
            this, &NavigatorController::onControlIndexChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::parentChanged,
            this, &NavigatorController::onControlParentChange);
}

Control* NavigatorController::controlFromItem(const QTreeWidgetItem* item) const
{
    return item->data(0, NavigatorDelegate::ControlRole).value<QPointer<Control>>().data();
}

QTreeWidgetItem* NavigatorController::itemFromControl(const Control* control) const
{
    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (controlFromItem(item) == control)
            return item;
    }
    return nullptr;
}

void NavigatorController::discharge()
{
    m_isProjectStarted = false;
    m_formStates.clear();
    m_navigatorPane->searchEdit()->clear();
    clear();
}

void NavigatorController::clear()
{
    m_isSelectionHandlingBlocked = true;
    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    EVERYTHING(QTreeWidgetItem* item, tree)
            tree->delegate()->destroyItem(item);
    m_searchCompleterModel.setStringList({});
    m_isSelectionHandlingBlocked = false;
}

void NavigatorController::onProjectStart()
{
    Q_ASSERT(!m_isProjectStarted);
    Q_ASSERT(m_designerScene->currentForm());
    m_isProjectStarted = true;
    onCurrentFormChange(m_designerScene->currentForm());
}

void NavigatorController::onSearchEditReturnPress()
{
    if (!m_isProjectStarted)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (const Control* control = controlFromItem(item)) {
            if (QString::compare(control->id(), m_navigatorPane->searchEdit()->text(), Qt::CaseInsensitive) == 0) {
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

void NavigatorController::onItemDoubleClick(QTreeWidgetItem* item, int)
{
    if (!m_isProjectStarted)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    m_isSelectionHandlingBlocked = true;
    tree->clearSelection();
    m_isSelectionHandlingBlocked = false;
    item->setSelected(true);
    tree->scrollToItem(item);

    const NavigatorSettings* settings = DesignerSettings::navigatorSettings();
    if (QGraphicsItem* mouseGrabber = m_designerScene->mouseGrabberItem())
        mouseGrabber->ungrabMouse();
    if (settings->itemDoubleClickAction == 0)
        emit editAnchorsActionTriggered();
    else if (settings->itemDoubleClickAction == 1)
        emit viewSourceCodeActionTriggered();
    else
        emit goToSlotActionTriggered();
}

void NavigatorController::onItemSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    const QList<QTreeWidgetItem*>& selectedItems = m_navigatorPane->navigatorTree()->selectedItems();

    QList<Control*> selectedControls;
    for (QTreeWidgetItem* item : selectedItems) {
        if (Control* control = controlFromItem(item))
            selectedControls.append(control);
    }

    m_isSelectionHandlingBlocked = true;
    emit controlSelectionChanged(selectedControls);
    m_isSelectionHandlingBlocked = false;
}

void NavigatorController::onSceneSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();
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

void NavigatorController::onControlCreation(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->scene() == 0)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

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

void NavigatorController::onControlRemove(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->type() == Form::Type)
        return;

    if (m_currentForm == 0)
        return;

    if (!m_currentForm->isAncestorOf(control))
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (const Control* ctrl = controlFromItem(item)) {
            if (ctrl == control) {
                const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(item);
                for (QTreeWidgetItem* child : childs) {
                    if (const Control* ctrl2 = controlFromItem(child)) {
                        removeCompleterEntry(ctrl2->id());
                        tree->delegate()->destroyItem(child);
                    }
                }
                // No need to sort, because the order is
                // already preserved after the deletion
                return;
            }
        }
    }
}

void NavigatorController::onFormRemove(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->type() != Form::Type)
        return;

    m_formStates.remove(static_cast<Form*>(control));

    // Form items will be cleared right after this slot,
    // when onCurrentFormChange slot is called.
}

void NavigatorController::onCurrentFormChange(Form* currentForm)
{
    if (!m_isProjectStarted)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

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

void NavigatorController::onControlParentChange(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (!control->parentControl())
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

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

void NavigatorController::onControlIndexChange(Control* control) const
{
    if (!m_isProjectStarted)
        return;

    if (control->type() == Form::Type)
        return;

    if (m_currentForm == 0)
        return;

    if (!m_currentForm->isAncestorOf(control))
        return;

    m_navigatorPane->navigatorTree()->sortItems(0, Qt::AscendingOrder);
}

void NavigatorController::onControlIdChange(Control* control, const QString& previousId)
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
    m_navigatorPane->navigatorTree()->update();
}

void NavigatorController::addCompleterEntry(const QString& entry)
{
    QStringList list(m_searchCompleterModel.stringList());
    list.append(entry);
    m_searchCompleterModel.setStringList(list);
}

void NavigatorController::removeCompleterEntry(const QString& entry)
{
    QStringList list(m_searchCompleterModel.stringList());
    list.removeOne(entry);
    m_searchCompleterModel.setStringList(list);
}

void NavigatorController::expandRecursive(const QTreeWidgetItem* parentItem)
{
    m_navigatorPane->navigatorTree()->expandItem(parentItem);
    for (int i = 0; i < parentItem->childCount(); ++i)
        expandRecursive(parentItem->child(i));
}

void NavigatorController::addControls(QTreeWidgetItem* parentItem, const QList<Control*>& controls)
{
    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    for (Control* control : controls) {
        QTreeWidgetItem* item = tree->delegate()->createItem(control);
        parentItem->addChild(item);
        addCompleterEntry(control->id());
        addControls(item, control->childControls(false));
    }
}
