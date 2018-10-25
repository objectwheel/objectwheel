#include <fontcolorssettings.h>
#include <codeeditorsettings.h>

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
    fontPreferThick = value<bool>(g_fontPreferThick, fontPreferThick);
    fontPreferAntialiasing = value<bool>(g_fontPreferAntialiasing, fontPreferAntialiasing);
    fontPixelSize = value<int>(g_fontPixelSize, fontPixelSize);
    fontFamily = value<QString>(g_fontFamily, fontFamily);
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

QFont FontColorsSettings::toFont() const
{
    QFont font(fontFamily);
    font.setPixelSize(fontPixelSize);
    font.setWeight(fontPreferThick ? QFont::Medium : QFont::Normal);
    font.setStyleStrategy(fontPreferAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    return font;
}

QTextCharFormat FontColorsSettings::toTextCharFormat(TextEditor::TextStyle category) const
{
// WARNING
}

QTextCharFormat FontColorsSettings::toTextCharFormat(TextEditor::TextStyles textStyles) const
{
    // WARNING
}