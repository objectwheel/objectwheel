#include <codeeditorsettings.h>
#include <fontcolorssettings.h>

CodeEditorSettings* CodeEditorSettings::s_instance = nullptr;
FontColorsSettings* CodeEditorSettings::s_fontColorsSettings = nullptr;

CodeEditorSettings::CodeEditorSettings(QObject* parent) : GroupSettings(parent)
{
    s_instance = this;
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

FontColorsSettings* CodeEditorSettings::fontColorsSettings()
{
    return s_fontColorsSettings;
}

const char* CodeEditorSettings::group() const
{
    return "CodeEditor";
}

void CodeEditorSettings::read()
{
    s_fontColorsSettings->read();
}

void CodeEditorSettings::write()
{
    s_fontColorsSettings->write();
}

void CodeEditorSettings::reset()
{
    s_fontColorsSettings->reset();
}