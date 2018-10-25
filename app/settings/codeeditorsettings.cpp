#include <codeeditorsettings.h>
#include <fontsettings.h>

namespace { const char* g_group = "General"; }

CodeEditorSettings* CodeEditorSettings::s_instance = nullptr;
FontSettings* CodeEditorSettings::s_fontSettings = nullptr;

CodeEditorSettings::CodeEditorSettings(QObject* parent) : QObject(parent)
{
    s_instance = this;
    s_fontSettings = new FontSettings(g_group, this);
}

CodeEditorSettings::~CodeEditorSettings()
{
    s_instance = nullptr;
}

CodeEditorSettings* CodeEditorSettings::instance()
{
    return s_instance;
}

FontSettings* CodeEditorSettings::fontSettings()
{
    return s_fontSettings;
}

void CodeEditorSettings::read()
{
    s_fontSettings->read();
}

void CodeEditorSettings::write()
{
    s_fontSettings->write();
}

void CodeEditorSettings::reset()
{
    s_fontSettings->reset();
}
