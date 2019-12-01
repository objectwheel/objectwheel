#include <modeselectorcontroller.h>
#include <modeselectorpane.h>
#include <QToolButton>

ModeSelectorController::ModeSelectorController(ModeSelectorPane* modeSelectorPane, QObject* parent) : QObject(parent)
  , m_modeSelectorPane(modeSelectorPane)
{
    onModeChange(ModeManager::mode());

    connect(ModeManager::instance(), &ModeManager::modeChanged,
            this, &ModeSelectorController::onModeChange);

    connect(m_modeSelectorPane->designerButton(), &QToolButton::clicked,
            this, &ModeSelectorController::onDesignerButtonClick);
    connect(m_modeSelectorPane->editorButton(), &QToolButton::clicked,
            this, &ModeSelectorController::onEditorButtonClick);
    connect(m_modeSelectorPane->splitButton(), &QToolButton::clicked,
            this, &ModeSelectorController::onSplitButtonClick);
    connect(m_modeSelectorPane->optionsButton(), &QToolButton::clicked,
            this, &ModeSelectorController::onOptionsButtonClick);
    connect(m_modeSelectorPane->buildsButton(), &QToolButton::clicked,
            this, &ModeSelectorController::onBuildsButtonClick);
    connect(m_modeSelectorPane->documentsButton(), &QToolButton::clicked,
            this, &ModeSelectorController::onDocumentsButtonClick);
}

void ModeSelectorController::discharge()
{
    ModeManager::setMode(ModeManager::Designer);
}

void ModeSelectorController::onModeChange(ModeManager::Mode mode)
{
    switch (mode) {
    case ModeManager::Designer:
        m_modeSelectorPane->designerButton()->setChecked(true);
        break;
    case ModeManager::Editor:
        m_modeSelectorPane->editorButton()->setChecked(true);
        break;
    case ModeManager::Split:
        m_modeSelectorPane->splitButton()->setChecked(true);
        break;
    case ModeManager::Options:
        m_modeSelectorPane->optionsButton()->setChecked(true);
        break;
    case ModeManager::Builds:
        m_modeSelectorPane->buildsButton()->setChecked(true);
        break;
    case ModeManager::Documents:
        m_modeSelectorPane->documentsButton()->setChecked(true);
        break;
    default:
        break;
    }
}

void ModeSelectorController::onDesignerButtonClick(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Designer);
}

void ModeSelectorController::onEditorButtonClick(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Editor);
}

void ModeSelectorController::onSplitButtonClick(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Split);
}

void ModeSelectorController::onOptionsButtonClick(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Options);
}

void ModeSelectorController::onBuildsButtonClick(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Builds);
}

void ModeSelectorController::onDocumentsButtonClick(bool checked)
{
    if (checked)
        ModeManager::setMode(ModeManager::Documents);
}
