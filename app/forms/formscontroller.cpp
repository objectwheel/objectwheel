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
#include <form.h>

#include <QPushButton>
#include <QCompleter>
#include <QTemporaryDir>

FormsController::FormsController(FormsPane* formsPane, DesignerScene* designerScene, QObject* parent) : QObject(parent)
  , m_formsPane(formsPane)
  , m_designerScene(designerScene)
  , m_isProjectStarted(false)
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
            this, &FormsController::refresh);
    connect(tree, &FormsTree::itemSelectionChanged,
            this, &FormsController::onItemSelectionChange);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &FormsController::onProjectStart);
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

void FormsController::onProjectStart()
{
    m_isProjectStarted = true;
    refresh();
}

void FormsController::discharge()
{
    m_isProjectStarted = false;
    blockSignals(true);
    m_formsPane->formsTree()->clear();
    blockSignals(false);
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

    refresh(); // FIXME: This function has severe performance issues.
}

void FormsController::onRemoveButtonClick()
{
    if (m_formsPane->formsTree()->topLevelItemCount() > 1) // FIXME
        ControlRemovingManager::removeControl(m_designerScene->currentForm(), true);
    // refresh(); // Not needed, m_designerScene already emits currentFormChanged signal
}

void FormsController::onItemSelectionChange()
{
    Q_ASSERT(m_formsPane->formsTree()->currentItem());

    const QString& id = m_formsPane->formsTree()->currentItem()->text(0);
    for (Form* form : m_designerScene->forms()) {
        if (form->id() == id)
            m_designerScene->setCurrentForm(form);
    }
}

void FormsController::refresh() // FIXME: This function has severe performance issues
{
    if (!m_isProjectStarted)
        return;

    blockSignals(true);

    m_formsPane->formsTree()->clear();

    QTreeWidgetItem* selectionItem = nullptr;
    // FIXME: Should we use scene->forms() instead? --but if you do, make sure you order forms with their indexes--
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

    if (selectionItem)
        selectionItem->setSelected(true);

    blockSignals(false);
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
