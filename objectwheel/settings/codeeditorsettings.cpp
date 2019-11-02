#include <codeeditorsettings.h>
#include <behaviorsettings.h>
#include <fontcolorssettings.h>

CodeEditorSettings* CodeEditorSettings::s_instance = nullptr;
BehaviorSettings* CodeEditorSettings::s_behaviorSettings = nullptr;
FontColorsSettings* CodeEditorSettings::s_fontColorsSettings = nullptr;

CodeEditorSettings::CodeEditorSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
    s_behaviorSettings = new BehaviorSettings(this);
    s_fontColorsSettings = new FontColorsSettings(this);
}

CodeEditorSettings::~CodeEditorSettings()
{
    s_instance = nullptr;
}

CodeEditorSettings* CodeEditorSettings::instance()
{
    return s_instance;
}

void CodeEditorSettings::read()
{
    s_behaviorSettings->read();
    s_fontColorsSettings->read();
}

void CodeEditorSettings::write()
{
    s_behaviorSettings->write();
    s_fontColorsSettings->write();
}

void CodeEditorSettings::reset()
{
    s_behaviorSettings->reset();
    s_fontColorsSettings->reset();
}

const char* CodeEditorSettings::group() const
{
    return "CodeEditor";
}

BehaviorSettings* CodeEditorSettings::behaviorSettings()
{
    return s_behaviorSettings;
}

FontColorsSettings* CodeEditorSettings::fontColorsSettings()
{
    return s_fontColorsSettings;
}