#include <codeeditorsettings.h>
#include <fontcolorssettings.h>

namespace { const char* g_group = "General"; }

CodeEditorSettings* CodeEditorSettings::s_instance = nullptr;
FontColorsSettings* CodeEditorSettings::s_fontColorsSettings = nullptr;

CodeEditorSettings::CodeEditorSettings(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_fontColorsSettings = new FontColorsSettings(g_group, this);
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
