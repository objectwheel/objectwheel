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

// FIXME: Make sure you check all currentForm() usages if they are null or not before using them

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

    // WARNING: Beware, ControlPropertyManager signals are emitted everytime a setProperty called
    // no matter what. I think we should consider reviewing related slots against possible miscalls
    connect(m_navigatorPane->searchEdit(), &LineEdit::editingFinished,
            this, &NavigatorController::onSearchEditEditingFinish);
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
            this, &NavigatorController::onControlRenderInfoChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &NavigatorController::onControlIdChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::indexChanged,
            this, &NavigatorController::onControlIndexChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::parentChanged,
            this, &NavigatorController::onControlParentChange);
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
    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem, true, true);
        for (QTreeWidgetItem* childItem : childs)
            tree->delegate()->destroyItem(childItem);
    }
    m_searchCompleterModel.setStringList({});
}

void NavigatorController::onProjectStart()
{
    Q_ASSERT(!m_isProjectStarted);
    m_isProjectStarted = true;
    onCurrentFormChange(m_designerScene->currentForm());
}

void NavigatorController::onSearchEditEditingFinish()
{
    if (!m_isProjectStarted)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    const QString& searchTerm = m_navigatorPane->searchEdit()->text();
    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            if (QString::compare(childItem->text(0), searchTerm, Qt::CaseInsensitive) == 0) {
                m_isSelectionHandlingBlocked = true;
                tree->clearSelection();
                m_isSelectionHandlingBlocked = false;
                childItem->setSelected(true);
                tree->scrollToItem(childItem);
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
        state.verticalScrollBarPosition = tree->verticalScrollBar()->sliderPosition();
        state.horizontalScrollBarPosition = tree->horizontalScrollBar()->sliderPosition();

        const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
        for (QTreeWidgetItem* topLevelItem : topLevelItems) {
            const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
            for (QTreeWidgetItem* childItem : childs) {
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

    /* Create items for incoming form */
    auto formItem = tree->delegate()->createItem();
    formItem->setText(0, currentForm->id());
    formItem->setData(0, NavigatorDelegate::HasErrorRole, currentForm->hasErrors());
    formItem->setData(1, NavigatorDelegate::HasErrorRole, currentForm->hasErrors());
    formItem->setIcon(0, QIcon(":/images/tools/Form.svg"));

    if (currentForm->gui() && !currentForm->hasErrors())
        formItem->setText(1, tr("Yes"));
    else
        formItem->setText(1, tr("No"));

    tree->addTopLevelItem(formItem);
    addCompleterEntry(m_searchCompleterModel, currentForm->id());
    appendChilds(formItem, currentForm->childControls(false));
    tree->sortItems(0, Qt::AscendingOrder);
    tree->expandAll();

    /* Restore incoming form's state */
    bool scrolled = false;
    const FormState& state = m_formStates.value(currentForm);
    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            for (const QString& id : state.selectedIds) {
                if (id == childItem->text(0)) {
                    childItem->setSelected(true);
                    if (!scrolled) {
                        tree->scrollToItem(childItem);
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
        const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
        for (QTreeWidgetItem* topLevelItem : topLevelItems) {
            const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
            for (QTreeWidgetItem* childItem : childs) {
                if (parentControl->id() == childItem->text(0)) {
                    appendChilds(childItem, QList<Control*>() << control);
                    expandAllChildren(tree, childItem);
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
            if (control->id() == childItem->text(0)) {
                const QList<QTreeWidgetItem*>& _childs = tree->allSubChildItems(childItem);
                for (QTreeWidgetItem* child : _childs) {
                    removeCompleterEntry(m_searchCompleterModel, child->text(0));
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
            if (control->id() == childItem->text(0))
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

void NavigatorController::onControlRenderInfoChange(Control* control, bool codeChanged)
{
    if (!m_isProjectStarted)
        return;

    if (!codeChanged)
        return;

    if (control->type() == Form::Type && m_designerScene->currentForm() != control)
        return;

    if (m_designerScene->currentForm() != control
            && !m_designerScene->currentForm()->isAncestorOf(control)) {
        return;
    }

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            if (control->id() == childItem->text(0)) {
                // No need to following, onControlIdChange takes place before the render operation
                // childItem->setText(0, control->id());
                // Hence no need to this too: sortItems(0, Qt::AscendingOrder);
                childItem->setData(0, NavigatorDelegate::HasErrorRole, control->hasErrors());
                childItem->setData(1, NavigatorDelegate::HasErrorRole, control->hasErrors());

                if (control->gui() && !control->hasErrors())
                    childItem->setText(1, tr("Yes"));
                else
                    childItem->setText(1, tr("No"));

                childItem->setIcon(0, QIcon(control->type() == Form::Type
                                            ? ":/images/tools/Form.svg"
                                            : ToolUtils::toolIconPath(control->dir())));
                return;
            }
        }
    }
}

void NavigatorController::appendChilds(QTreeWidgetItem* parentItem, const QList<Control*>& childItems)
{
    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    for (const Control* child : childItems) {
        QTreeWidgetItem* item = tree->delegate()->createItem();
        item->setText(0, child->id());
        item->setData(0, NavigatorDelegate::HasErrorRole, child->hasErrors());
        item->setData(1, NavigatorDelegate::HasErrorRole, child->hasErrors());
        item->setExpanded(true);

        if (child->gui() && !child->hasErrors())
            item->setText(1, QObject::tr("Yes"));
        else
            item->setText(1, QObject::tr("No"));

        item->setIcon(0, QIcon(ToolUtils::toolIconPath(child->dir())));
        parentItem->addChild(item);
        addCompleterEntry(m_searchCompleterModel, child->id());
        appendChilds(item, child->childControls(false));
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

    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
    for (QTreeWidgetItem* topLevelItem : topLevelItems) {
        const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
        for (QTreeWidgetItem* childItem : childs) {
            if (previousId == childItem->text(0)) {
                childItem->setText(0, control->id());
                removeCompleterEntry(m_searchCompleterModel, previousId);
                addCompleterEntry(m_searchCompleterModel, control->id());
            }
        }
    }
}

void NavigatorController::onSceneSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    m_isSelectionHandlingBlocked = true;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();
    tree->clearSelection();

    bool scrolled = false;
    for (const Control* selectedControl : m_designerScene->selectedControls()) {
        const QList<QTreeWidgetItem*>& topLevelItems = tree->topLevelItems();
        for (QTreeWidgetItem* topLevelItem : topLevelItems) {
            const QList<QTreeWidgetItem*>& childs = tree->allSubChildItems(topLevelItem);
            for (QTreeWidgetItem* childItem : childs) {
                if (selectedControl->id() == childItem->text(0)) {
                    childItem->setSelected(true);
                    if (!scrolled) {
                        tree->scrollToItem(childItem);
                        scrolled = true;
                    }
                }
            }
        }
    }

    m_isSelectionHandlingBlocked = false;
}

void NavigatorController::onItemSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    NavigatorTree* tree = m_navigatorPane->navigatorTree();

    QList<Control*> selectedControls;
    for (QTreeWidgetItem* item : tree->selectedItems()) {
        Control* control = controlFromItem(item, m_designerScene->currentForm());
        if (!control)
            continue;

        selectedControls.append(control);
    }

    m_isSelectionHandlingBlocked = true;
    emit controlSelectionChanged(selectedControls);
    m_isSelectionHandlingBlocked = false;
}
