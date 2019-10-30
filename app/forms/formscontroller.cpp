#include <formscontroller.h>
#include <formsdelegate.h>
#include <formstree.h>
#include <formspane.h>
#include <lineedit.h>
#include <designerscene.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <filesystemutils.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <controlpropertymanager.h>
#include <form.h>

#include <QPushButton>
#include <QCompleter>
#include <QTemporaryDir>

FormsController::FormsController(FormsPane* formsPane, DesignerScene* designerScene, QObject* parent) : QObject(parent)
  , m_formsPane(formsPane)
  , m_designerScene(designerScene)
  , m_isProjectStarted(false)
  , m_isSelectionHandlingBlocked(false)
{
    FormsTree* tree = m_formsPane->formsTree();

    auto completer = new QCompleter(this);
    completer->setModel(&m_searchCompleterModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    m_formsPane->searchEdit()->setCompleter(completer);

    connect(m_formsPane->searchEdit(), &LineEdit::returnPressed,
            this, &FormsController::onSearchEditReturnPress);
    connect(formsPane->addButton(), &QPushButton::clicked,
            this, &FormsController::onAddButtonClick);
    connect(formsPane->removeButton(), &QPushButton::clicked,
            this, &FormsController::onRemoveButtonClick);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &FormsController::onCurrentFormChange);
    connect(tree, &FormsTree::itemSelectionChanged,
            this, &FormsController::onItemSelectionChange);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &FormsController::onProjectStart);
    connect(ControlCreationManager::instance(), &ControlCreationManager::controlCreated,
            this, &FormsController::onControlCreation);
    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
            this, &FormsController::onControlRemove);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
            tree, qOverload<>(&FormsTree::update));
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
            this, &FormsController::onControlIdChange);
    connect(ControlPropertyManager::instance(), &ControlPropertyManager::indexChanged,
            this, &FormsController::onControlIndexChange);
}

Control* FormsController::controlFromItem(const QTreeWidgetItem* item) const
{
    return item->data(0, FormsDelegate::ControlRole).value<QPointer<Control>>().data();
}

QTreeWidgetItem* FormsController::itemFromControl(const Control* control) const
{
    FormsTree* tree = m_formsPane->formsTree();
    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (controlFromItem(item) == control)
            return item;
    }
    return nullptr;
}

void FormsController::discharge()
{
    m_isProjectStarted = false;
    clear();
}

void FormsController::clear()
{
    m_isSelectionHandlingBlocked = true;
    m_formsPane->formsTree()->clear();
    m_searchCompleterModel.setStringList({});
    m_isSelectionHandlingBlocked = false;
}

void FormsController::onProjectStart()
{
    m_isProjectStarted = true;




    if (!m_isProjectStarted)
        return;

    clear();

    // FIXME: Should we use scene->forms() instead? --but if
    // you do, make sure you order forms with their indexes--
    QTreeWidgetItem* selectionItem = nullptr;
    for (const QString& path : SaveUtils::formPaths(ProjectManager::dir())) {
        const QString& id = SaveUtils::controlId(path);
        Q_ASSERT(!id.isEmpty());

        auto item = new QTreeWidgetItem;
        item->setText(0, id);
        item->setIcon(0, QIcon(":/images/designer/form.svg"));

        m_formsPane->formsTree()->addTopLevelItem(item);

        if (m_designerScene->currentForm() && m_designerScene->currentForm()->id() == id)
            selectionItem = item;
    }

    m_isSelectionHandlingBlocked = true;
    if (selectionItem)
        selectionItem->setSelected(true);
    m_isSelectionHandlingBlocked = false;
}

void FormsController::onAddButtonClick()
{
    QTemporaryDir temp;
    Q_ASSERT(temp.isValid());

    SaveUtils::initControlMeta(temp.path());

    const QString& thisDir = SaveUtils::toControlThisDir(temp.path());

    QDir(thisDir).mkpath(".");
    FileSystemUtils::copy(":/resources/qmls/form.qml", thisDir, true, true);
    QFile::rename(thisDir + "/form.qml", thisDir + '/' + SaveUtils::controlMainQmlFileName());
    ControlCreationManager::createForm(temp.path());
    // onControlCreation(); Not needed, ControlCreationManager::controlCreated will be emitted
}

void FormsController::onRemoveButtonClick()
{
    if (m_formsPane->formsTree()->topLevelItemCount() > 1) // FIXME
        ControlRemovingManager::removeControl(m_designerScene->currentForm(), true);
    // onControlRemove(); Not needed, ControlRemovingManager::controlAboutToBeRemoved will be emitted
}

void FormsController::onSearchEditReturnPress()
{
    if (!m_isProjectStarted)
        return;

    FormsTree* tree = m_formsPane->formsTree();

    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (const Control* control = controlFromItem(item)) {
            if (QString::compare(control->id(), m_formsPane->searchEdit()->text(), Qt::CaseInsensitive) == 0) {
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

void FormsController::onControlCreation(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->type() != Form::Type)
        return;

    FormsTree* tree = m_formsPane->formsTree();
    QTreeWidgetItem* item = tree->delegate()->createItem(control);
    tree->addTopLevelItem(item);
    addCompleterEntry(control->id());
    tree->sortItems(0, Qt::AscendingOrder);
}

void FormsController::onControlRemove(Control* control)
{
    if (!m_isProjectStarted)
        return;

    if (control->type() != Form::Type)
        return;

    FormsTree* tree = m_formsPane->formsTree();

    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (const Control* ctrl = controlFromItem(item)) {
            if (ctrl == control) {
                removeCompleterEntry(ctrl->id());
                m_isSelectionHandlingBlocked = true;
                tree->delegate()->destroyItem(item);
                m_isSelectionHandlingBlocked = false;
                // No need to sort, because the order is
                // already preserved after the deletion
                return;
            }
        }
    }
}

void FormsController::onControlIdChange(Control* control, const QString& previousId)
{
    if (!m_isProjectStarted)
        return;

    if (previousId.isEmpty())
        return;

    if (control->id() == previousId)
        return;

    if (control->type() != Form::Type)
        return;

    removeCompleterEntry(previousId);
    addCompleterEntry(control->id());
    m_formsPane->formsTree()->update();
}

void FormsController::onControlIndexChange(Control* control) const
{
    if (!m_isProjectStarted)
        return;

    if (control->type() != Form::Type)
        return;

    m_formsPane->formsTree()->sortItems(0, Qt::AscendingOrder);
}

void FormsController::onCurrentFormChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    if (m_designerScene->currentForm() == 0)
        return;

    FormsTree* tree = m_formsPane->formsTree();

    EVERYTHING(QTreeWidgetItem* item, tree) {
        if (const Control* control = controlFromItem(item)) {
            if (control == m_designerScene->currentForm()) {
                m_isSelectionHandlingBlocked = true;
                tree->clearSelection();
                item->setSelected(true);
                m_isSelectionHandlingBlocked = false;
                tree->scrollToItem(item);
                return;
            }
        }
    }
}

void FormsController::onItemSelectionChange()
{
    if (!m_isProjectStarted)
        return;

    if (m_isSelectionHandlingBlocked)
        return;

    FormsTree* tree = m_formsPane->formsTree();
    Q_ASSERT(tree->currentItem());

    if (auto control = controlFromItem(tree->currentItem())) {
        const QList<Form*>& forms = m_designerScene->forms();
        for (Form* form : forms) {
            if (form == control) {
                m_isSelectionHandlingBlocked = true;
                m_designerScene->setCurrentForm(form);
                m_isSelectionHandlingBlocked = false;
            }
        }
    }
}

void FormsController::addCompleterEntry(const QString& entry)
{
    QStringList list(m_searchCompleterModel.stringList());
    list.append(entry);
    m_searchCompleterModel.setStringList(list);
}

void FormsController::removeCompleterEntry(const QString& entry)
{
    QStringList list(m_searchCompleterModel.stringList());
    list.removeOne(entry);
    m_searchCompleterModel.setStringList(list);
}
