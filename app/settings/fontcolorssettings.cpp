#include <fontcolorssettings.h>
#include <codeeditorsettings.h>
#include <QVariant>

namespace {
const char* g_fontFamily = "FontFamily";
const char* g_fontPixelSize = "FontPixelSize";
const char* g_fontPreferThick = "FontPreferThick";
const char* g_fontPreferAntialiasing = "FontPreferAntialiasing";
}

FontColorsSettings::FontColorsSettings(CodeEditorSettings* codeEditorSettings) : Settings(codeEditorSettings)
{
    reset();
}

void FontColorsSettings::read()
{
    reset();

    begin();
    fontPreferThick = value(g_fontPreferThick, fontPreferThick).value<bool>();
    fontPreferAntialiasing = value(g_fontPreferAntialiasing, fontPreferAntialiasing).value<bool>();
    fontPixelSize = value(g_fontPixelSize, fontPixelSize).value<int>();
    fontFamily = value(g_fontFamily, fontFamily).value<QString>();
    end();
}

void FontColorsSettings::write()
{
    begin();
    setValue(g_fontPreferThick, fontPreferThick);
    setValue(g_fontPreferAntialiasing, fontPreferAntialiasing);
    setValue(g_fontPixelSize, fontPixelSize);
    setValue(g_fontFamily, fontFamily);
    end();

    emit static_cast<CodeEditorSettings*>(groupSettings())->fontColorsSettingsChanged();
}

void FontColorsSettings::reset()
{
    fontPreferThick = false;
    fontPreferAntialiasing = true;
    fontPixelSize = 13;
#if defined(Q_OS_MACOS)
    fontFamily = ".SF NS Display";
#elif defined(Q_OS_WIN)
    fontFamily = "Segoe UI";
#else
    fontFamily = "Roboto";
#endif
}

const char* FontColorsSettings::category() const
{
    return "FontColors";
}
