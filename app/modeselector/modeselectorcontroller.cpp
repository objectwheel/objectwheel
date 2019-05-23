#include <modeselectorcontroller.h>
#include <modeselectorpane.h>
#include <projectmanager.h>

ModeSelectorController::ModeSelectorController(ModeSelectorPane* modeSelectorPane, QObject* parent) : QObject(parent)
  , m_modeSelectorPane(modeSelectorPane)
{
    m_modeSelectorPane->designerAction()->setChecked(true);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &ModeSelectorController::discharge);

    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, &ModeSelectorController::onModeChange);

    connect(m_modeSelectorPane->designerAction(), &QAction::triggered,
            this, &ModeSelectorController::onDesignerActionTriggered);
    connect(m_modeSelectorPane->editorAction(), &QAction::triggered,
            this, &ModeSelectorController::onEditorActionTriggered);
    connect(m_modeSelectorPane->splitAction(), &QAction::triggered,
            this, &ModeSelectorController::onSplitActionTriggered);
    connect(m_modeSelectorPane->optionsAction(), &QAction::triggered,
            this, &ModeSelectorController::onOptionsActionTriggered);
    connect(m_modeSelectorPane->buildsAction(), &QAction::triggered,
            this, &ModeSelectorController::onBuildsActionTriggered);
    connect(m_modeSelectorPane->documentsAction(), &QAction::triggered,
            this, &ModeSelectorController::onDocumentsActionTriggered);
}

void ModeSelectorController::discharge()
{
    ModeManager::setMode(ModeManager::Designer);
}

void ModeSelectorController::onModeChange(ModeManager::Mode mode)
{
    switch (mode) {
    case ModeManager::Designer:
        m_modeSelectorPane->designerAction()->setChecked(true);
        break;
    case ModeManager::Editor:
        m_modeSelectorPane->editorAction()->setChecked(true);
        break;
    case ModeManager::Split:
        m_modeSelectorPane->splitAction()->setChecked(true);
        break;
    case ModeManager::Options:
        m_modeSelectorPane->optionsAction()->setChecked(true);
        break;
    case ModeManager::Builds:
        m_modeSelectorPane->buildsAction()->setChecked(true);
        break;
    case ModeManager::Documents:
        m_modeSelectorPane->documentsAction()->setChecked(true);
        break;
    default:
        break;
    }
}

void ModeSelectorController::onDesignerActionTriggered(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Designer);
}

void ModeSelectorController::onEditorActionTriggered(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Editor);
}

void ModeSelectorController::onSplitActionTriggered(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Split);
}

void ModeSelectorController::onOptionsActionTriggered(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Options);
}

void ModeSelectorController::onBuildsActionTriggered(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Builds);
}

void ModeSelectorController::onDocumentsActionTriggered(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Documents);
}
