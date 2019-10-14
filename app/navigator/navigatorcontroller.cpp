#include <navigatorcontroller.h>
#include <navigatorpane.h>
#include <navigatortree.h>
#include <navigatordelegate.h>
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
#include <lineedit.h>

#include <QScrollBar>
#include <QCompleter>

static void addCompleterEntry(QStringListModel& model, const QString& entry)
{
    QStringList list(model.stringList());
    list.append(entry);
    model.setStringList(list);
}

static void removeCompleterEntry(QStringListModel& model, const QString& entry)
{
    QStringList list(model.stringList());
    list.removeOne(entry);
    model.setStringList(list);
}

static void expandAllChildren(QTreeWidget* treeWidget, QTreeWidgetItem* parentItem)
{
    treeWidget->expandItem(parentItem);
    for (int i = 0; i < parentItem->childCount(); ++i)
        expandAllChildren(treeWidget, parentItem->child(i));
}

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
                break;
            }
        }
    }
}

void NavigatorController::onCurrentFormChange(Form* currentForm)
{
    if (!m_isProjectStarted)
        return;

    if (!currentForm)
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

    /* Create items for incoming form */
    auto formItem = tree->delegate()->createItem(m_currentForm);
    addCompleterEntry(m_searchCompleterModel, currentForm->id());
    addControls(formItem, currentForm->childControls(false));

    tree->addTopLevelItem(formItem);
    tree->sortItems(0, Qt::AscendingOrder);
    tree->expandAll();

    /* Restore incoming form's state */
    QTreeWidgetItem* firstItem = nullptr;
    const QList<Control*>& selectedControls = m_designerScene->selectedControls();
    const FormState& state = m_formStates.value(currentForm);

    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (Control* control = controlFromItem(item)) {
            if (selectedControls.contains(control)) {
                item->setSelected(true);
                if (firstItem == 0)
                    firstItem = item;
            }
            if (state.collapsedControls.contains(control))
                item->setExpanded(false);
        }
    }

    tree->scrollToItem(firstItem);
    tree->verticalScrollBar()->setSliderPosition(state.verticalScrollBarPosition);
    tree->horizontalScrollBar()->setSliderPosition(state.horizontalScrollBarPosition);
}

void NavigatorController::onFormRemove(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->type() != Form::Type)
        return;

    m_formStates.remove(static_cast<Form*>(control));
}

// FIXME: FFFF
void NavigatorController::onControlCreation(Control* control)
{
    if (!m_isProjectStarted)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    if (const Control* parentControl = control->parentControl()) {
        EVERYTHING(QTreeWidgetItem* item, tree) {
            if (const Control* ctrl = controlFromItem(item)) {
                if (ctrl == parentControl) {
                    addControls(item, QList<Control*>{control});
                    expandAllChildren(tree, item);
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

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            const auto& control2 = childItem->data(0, NavigatorDelegate::ControlRole).value<QPointer<Control>>();
            Q_ASSERT(control2);
            if (control2.isNull())
                continue;
            if (control->id() == control2->id()) {
                const QList<QTreeWidgetItem*>& _childs = tree->allSubChildItems(childItem);
                for (QTreeWidgetItem* child : _childs) {
                    const auto& control3 = childItem->data(0, NavigatorDelegate::ControlRole).value<QPointer<Control>>();
                    Q_ASSERT(control3);
                    if (control3.isNull())
                        continue;
                    removeCompleterEntry(m_searchCompleterModel, control3->id());
                    tree->delegate()->destroyItem(child);
                }
                // No need to following, because the order is preserved after the deletion already.
                // sortItems(0, Qt::AscendingOrder);
                return;
            }
        }
    }
}

// FIXME: FFFF
void NavigatorController::onControlParentChange(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (!control->parentControl())
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    // Check if already exists, if not --reparented from another form-- add it to the list
    bool found = false;
    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            const auto& control2 = childItem->data(0, NavigatorDelegate::ControlRole).value<QPointer<Control>>();
            Q_ASSERT(control2);
            if (control2.isNull())
                continue;
            if (control->id() == control2->id())
                found = true;
        }
    }

    if (!found)
        return onControlCreation(control);

    const QList<QTreeWidgetItem*>& topLevelItems2 = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems2) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            if (control->id() == childItem->text(0)) {
                const QList<QTreeWidgetItem*>& topLevelItems3 = tree->topLevelItems();
                for (QTreeWidgetItem* topLevelItem_2 : topLevelItems3) {
                    const QList<QTreeWidgetItem*>& childs2 = tree->allSubChildItems(topLevelItem_2);
                    for (QTreeWidgetItem* childItem_2 : childs2) {
                        if (control->parentControl()->id() == childItem_2->text(0)) {
                            bool isExpanded = childItem->isExpanded();
                            childItem->parent()->removeChild(childItem);
                            childItem_2->addChild(childItem);
                            childItem->setExpanded(isExpanded);
                            tree->sortItems(0, Qt::AscendingOrder);
                            return;
                        }
                    }
                }
                return;
            }
        }
    }
}

// FIXME: FFFF
void NavigatorController::onControlIndexChange(Control* control)
{
    //    if (!m_isProjectStarted)
    //        return;

    //    if (control->form())
    //        return;

    //    if (!m_designerScene->currentForm()->isAncestorOf(control))
    //        return;

    //    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
    //        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
    //            if (previousId == childItem->text(0)) {
    //                childItem->setText(0, control->id());
    //                sortItems(0, Qt::AscendingOrder);
    //                goto phase2;
    //            }
    //        }
    //    }

    //phase2:

    //    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
    //        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
    //            if (childItem->isSelected()) {
    //                scrollToItem(childItem);
    //                return;
    //            }
    //        }
    //    }
}

void NavigatorController::onControlIdChange(Control* control, const QString& previousId)
{
    if (!m_isProjectStarted)
        return;

    if (previousId.isEmpty())
        return;

    if (control->id() == previousId)
        return;

    if (control->type() == Form::Type && m_designerScene->currentForm() != control)
        return;

    if (m_designerScene->currentForm() != control
            && !m_designerScene->currentForm()->isAncestorOf(control)) {
        return;
    }

    removeCompleterEntry(m_searchCompleterModel, previousId);
    addCompleterEntry(m_searchCompleterModel, control->id());
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

    QTreeWidgetItem* firstItem = nullptr;
    for (const Control* selectedControl : selectedControls) {
        if (QTreeWidgetItem* item = itemFromControl(selectedControl)) {
            item->setSelected(true);
            if (firstItem == 0)
                firstItem = item;
        }
    }

    tree->scrollToItem(firstItem);

    m_isSelectionHandlingBlocked = false;
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

void NavigatorController::addControls(QTreeWidgetItem* parentItem, const QList<Control*>& controls)
{
    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    for (Control* control : controls) {
        QTreeWidgetItem* item = tree->delegate()->createItem(control);
        item->setExpanded(true);
        parentItem->addChild(item);
        addCompleterEntry(m_searchCompleterModel, control->id());
        addControls(item, control->childControls(false));
    }
}
