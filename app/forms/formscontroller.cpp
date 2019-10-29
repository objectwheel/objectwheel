#include <formscontroller.h>
#include <formspane.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <designerscene.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <filesystemutils.h>
#include <form.h>
#include <applicationstyle.h>
#include <lineedit.h>

#include <QPushButton>
#include <QStandardPaths>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QDir>
#include <QTemporaryDir>
#include <QApplication>
#include <QCompleter>

//{

//    connect(tree, &ControlsTree::itemDoubleClicked,
//            this, &ControlsController::onItemDoubleClick);
//    connect(tree, &ControlsTree::itemSelectionChanged,
//            this, &ControlsController::onItemSelectionChange);
//    connect(m_designerScene, &DesignerScene::currentFormChanged,
//            this, &ControlsController::onCurrentFormChange);
//    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
//            this, &ControlsController::onControlRemove);
//    connect(ControlRemovingManager::instance(), &ControlRemovingManager::controlAboutToBeRemoved,
//            this, &ControlsController::onFormRemove);
//    connect(ProjectManager::instance(), &ProjectManager::started,
//            this, &ControlsController::onProjectStart);
//    connect(m_designerScene, &DesignerScene::selectionChanged,
//            this, &ControlsController::onSceneSelectionChange);
//    connect(ControlPropertyManager::instance(), &ControlPropertyManager::renderInfoChanged,
//            tree, qOverload<>(&ControlsTree::update));
//    connect(ControlPropertyManager::instance(), &ControlPropertyManager::idChanged,
//            this, &ControlsController::onControlIdChange);
//    connect(ControlPropertyManager::instance(), &ControlPropertyManager::indexChanged,
//            this, &ControlsController::onControlIndexChange);
//    connect(ControlPropertyManager::instance(), &ControlPropertyManager::parentChanged,
//            this, &ControlsController::onControlParentChange);
//    }

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
            this, &FormsPane::onAddButtonClick);
    connect(formsPane->removeButton(), &QPushButton::clicked,
            this, &FormsPane::onRemoveButtonClick);
    connect(m_designerScene, &DesignerScene::currentFormChanged,
            this, &FormsPane::refresh);
    connect(this, &FormsPane::currentItemChanged,
            this, &FormsPane::onCurrentItemChange);
    connect(ProjectManager::instance(), &ProjectManager::started,
            this, [=] {
        Q_ASSERT(!isProjectStarted);
        isProjectStarted = true;
        refresh(); // FIXME: This function has severe performance issues.
    });
}

void FormsPane::discharge()
{
    isProjectStarted = false;
    blockSignals(true);
    clear();
    blockSignals(false);
}

void FormsPane::onAddButtonClick()
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

void FormsPane::onRemoveButtonClick()
{
    if (topLevelItemCount() > 1) // FIXME
        ControlRemovingManager::removeControl(m_designerScene->currentForm(), true);
    // refresh(); // Not needed, m_designerScene already emits currentFormChanged signal
}

void FormsPane::onCurrentItemChange()
{
    Q_ASSERT(currentItem());

    const QString& id = currentItem()->text(0);
    for (Form* form : m_designerScene->forms()) {
        if (form->id() == id)
            m_designerScene->setCurrentForm(form);
    }
}

void FormsPane::refresh() // FIXME: This function has severe performance issues
{
    if (!isProjectStarted)
        return;

    blockSignals(true);

    clear();

    QTreeWidgetItem* selectionItem = nullptr;
    // FIXME: Should we use scene->forms() instead? --but if you do, make sure you order forms with their indexes--
    for (const QString& path : SaveUtils::formPaths(ProjectManager::dir())) {
        const QString& id = SaveUtils::controlId(path);
        Q_ASSERT(!id.isEmpty());

        auto item = new QTreeWidgetItem;
        item->setText(0, id);
        item->setIcon(0, QIcon(":/images/designer/form.svg"));

        addTopLevelItem(item);

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
