#include <behaviorsettings.h>
#include <codeeditorsettings.h>

namespace {
const char* g_autoSaveBeforeRunning = "AutoSaveBeforeRunning";
}

BehaviorSettings::BehaviorSettings(CodeEditorSettings* codeEditorSettings) : Settings(codeEditorSettings)
{
    reset();
}

void BehaviorSettings::read()
{
    reset();

    begin();
    autoSaveBeforeRunning = value<bool>(g_autoSaveBeforeRunning, autoSaveBeforeRunning);
    end();
}

void BehaviorSettings::write()
{
    begin();
    setValue(g_autoSaveBeforeRunning, autoSaveBeforeRunning);
    end();

    emit static_cast<CodeEditorSettings*>(groupSettings())->behaviorSettingsChanged();
}

void BehaviorSettings::reset()
{
    autoSaveBeforeRunning = true;
}

const char* BehaviorSettings::category() const
{
    return "BehaviorSettings";
}