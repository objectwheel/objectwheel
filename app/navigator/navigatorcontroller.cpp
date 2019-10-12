#include <navigatorcontroller.h>
#include <navigatorpane.h>
#include <navigatortree.h>
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

// FIXME: Make sure you check all currentForm() usages if they are null or not before using them

bool isProjectStarted = false;
bool isSelectionHandlingBlocked = false;

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

        item->setIcon(0, QIcon(ToolUtils::toolIconPath(child->dir())));
        parentItem->addChild(item);
        addChildrenIntoItem(item, child->childControls(false));
    }
}

NavigatorController::NavigatorController(NavigatorPane* navigatorPane, DesignerScene* designerScene, QObject* parent) : QObject(parent)
  , m_navigatorPane(navigatorPane)
  , m_designerScene(designerScene)
{
    // WARNING: Beware, ControlPropertyManager signals are emitted everytime a setProperty called
    // no matter what. I think we should consider reviewing related slots against possible miscalls
    connect(m_navigatorPane->navigatorTree(), &NavigatorTree::itemSelectionChanged,
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
    isProjectStarted = false;
    m_formStates.clear();
    clear();
}

void NavigatorController::clear()
{

}

void NavigatorController::onProjectStart()
{
    Q_ASSERT(!isProjectStarted);
    isProjectStarted = true;
    onCurrentFormChange(m_designerScene->currentForm());
}

void NavigatorController::onCurrentFormChange(Form* currentForm)
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

    /* Create items for incoming form */
    auto formItem = new QTreeWidgetItem;
    formItem->setText(0, currentForm->id());
    formItem->setData(0, Qt::UserRole, currentForm->hasErrors());
    formItem->setData(1, Qt::UserRole, currentForm->hasErrors());
    formItem->setIcon(0, QIcon(":/images/tools/Form.svg"));

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

    m_navigatorPane->navigatorTree()->verticalScrollBar()->setSliderPosition(state.verticalScrollBarPosition);
    m_navigatorPane->navigatorTree()->horizontalScrollBar()->setSliderPosition(state.horizontalScrollBarPosition);
}

void NavigatorController::onFormRemove(Control* control)
{
    if (!isProjectStarted)
        return;

    if (control->type() != Form::Type)
        return;

    m_formStates.remove(static_cast<Form*>(control));
}

// FIXME: FFFF
void NavigatorController::onControlCreation(Control* control)
{
    if (!isProjectStarted)
        return;

    if (const Control* parentControl = control->parentControl()) {
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
}

void NavigatorController::onControlRemove(Control* control)
{
    if (!isProjectStarted)
        return;

    if (control->type() == Form::Type)
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

// FIXME: FFFF
void NavigatorController::onControlParentChange(Control* control)
{
    if (!isProjectStarted)
        return;

    if (!control->parentControl())
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

void NavigatorController::onControlRenderInfoChange(Control* control, bool codeChanged)
{
    if (!isProjectStarted)
        return;

    if (!codeChanged)
        return;

    if (control->type() == Form::Type && m_designerScene->currentForm() != control)
        return;

    if (m_designerScene->currentForm() != control
            && !m_designerScene->currentForm()->isAncestorOf(control)) {
        return;
    }

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (control->id() == childItem->text(0)) {
                // No need to following, onControlIdChange takes place before the render operation
                // childItem->setText(0, control->id());
                // Hence no need to this too: sortItems(0, Qt::AscendingOrder);
                childItem->setData(0, Qt::UserRole, control->hasErrors());
                childItem->setData(1, Qt::UserRole, control->hasErrors());

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

// FIXME: FFFF
void NavigatorController::onControlIndexChange(Control* control)
{
//    if (!isProjectStarted)
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
//                scrollToItem(childItem, QAbstractItemView::PositionAtCenter);
//                return;
//            }
//        }
//    }
}

void NavigatorController::onControlIdChange(Control* control, const QString& previousId)
{
    if (!isProjectStarted)
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

    for (QTreeWidgetItem* topLevelItem : topLevelItems(this)) {
        for (QTreeWidgetItem* childItem : allSubChildItems(topLevelItem)) {
            if (previousId == childItem->text(0))
                childItem->setText(0, control->id());
        }
    }
}

void NavigatorController::onSceneSelectionChange()
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

void NavigatorController::onItemSelectionChange()
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
