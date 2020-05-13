#include <updatesettings.h>
#include <generalsettings.h>
#include <utilityfunctions.h>

static const char g_theme[] = "Theme";
static const char g_fontFamily[] = "FontFamily";
static const char g_fontPixelSize[] = "FontPixelSize";
static const char g_fontPreferThick[] = "FontPreferThick";
static const char g_fontPreferAntialiasing[] = "FontPreferAntialiasing";
static const char g_language[] = "Language";
static const char g_hdpiEnabled[] = "HdpiEnabled";
static const char g_highlightColor[] = "HighlightColor";
static const char g_outputPanePops[] = "OutputPanePops";
static const char g_preserveDesignerState[] = "PreserveDesignerState";
static const char g_outputPaneMinimizedStartupEnabled[] = "OutputPaneMinimizedStartupEnabled";

UpdateSettings::UpdateSettings(GeneralSettings* generalSettings) : Settings(generalSettings)
{
    reset();
}

void UpdateSettings::read()
{
    reset();

    begin();
    hdpiEnabled = value<bool>(g_hdpiEnabled, hdpiEnabled);
    theme = value<int>(g_theme, theme);
    language = value<int>(g_language, language);
    highlightColor = value<QColor>(g_highlightColor, highlightColor);
    /****/
    fontPreferThick = value<bool>(g_fontPreferThick, fontPreferThick);
    fontPreferAntialiasing = value<bool>(g_fontPreferAntialiasing, fontPreferAntialiasing);
    fontPixelSize = value<int>(g_fontPixelSize, fontPixelSize);
    fontFamily = value<QString>(g_fontFamily, fontFamily);
    /****/
    outputPanePops = value<bool>(g_outputPanePops, outputPanePops);
    preserveDesignerState = value<bool>(g_preserveDesignerState, preserveDesignerState);
    outputPaneMinimizedStartupEnabled = value<bool>(g_outputPaneMinimizedStartupEnabled, outputPaneMinimizedStartupEnabled);
    end();
}

void UpdateSettings::write()
{
    begin();
    setValue(g_hdpiEnabled, hdpiEnabled);
    setValue(g_theme, theme);
    setValue(g_language, language);
    setValue(g_highlightColor, highlightColor);
    /****/
    setValue(g_fontPreferThick, fontPreferThick);
    setValue(g_fontPreferAntialiasing, fontPreferAntialiasing);
    setValue(g_fontPixelSize, fontPixelSize);
    setValue(g_fontFamily, fontFamily);
    /****/
    setValue(g_outputPanePops, outputPanePops);
    setValue(g_preserveDesignerState, preserveDesignerState);
    setValue(g_outputPaneMinimizedStartupEnabled, outputPaneMinimizedStartupEnabled);
    end();

    emit static_cast<GeneralSettings*>(groupSettings())->updateSettingsChanged();
}

void UpdateSettings::reset()
{
    hdpiEnabled = true;
    theme = 0;
    language = 0;
    highlightColor = "#2483ec";
    /****/
    const QFont& df = UtilityFunctions::defaultFont();
    fontPreferThick = false;
    fontPreferAntialiasing = true;
    fontPixelSize = df.pixelSize();
    fontFamily = df.family();
    /****/
    outputPanePops = true;
    preserveDesignerState = true;
    outputPaneMinimizedStartupEnabled = true;
}

const char* UpdateSettings::category() const
{
    return "Update";
}

QFont UpdateSettings::toFont() const
{
    QFont font;
    font.setFamily(fontFamily);
    font.setPixelSize(fontPixelSize);
    font.setWeight(fontPreferThick ? QFont::Medium : QFont::Normal);
    font.setStyleStrategy(fontPreferAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    return font;
}
