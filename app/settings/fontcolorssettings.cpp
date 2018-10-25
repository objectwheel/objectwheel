#include <fontcolorssettings.h>
#include <codeeditorsettings.h>

namespace {

const char* g_fontFamily = "FontFamily";
const char* g_fontPixelSize = "FontPixelSize";
const char* g_fontPreferThick = "FontPreferThick";
const char* g_fontPreferAntialiasing = "FontPreferAntialiasing";

double clamp(double value)
{
    return std::max(0.0, std::min(1.0, value));
}

QBrush mixBrush(const QBrush &original, double relativeSaturation, double relativeLightness)
{
    const QColor originalColor = original.color().toHsl();
    QColor mixedColor(QColor::Hsl);

    double mixedSaturation = clamp(originalColor.hslSaturationF() + relativeSaturation);

    double mixedLightness = clamp(originalColor.lightnessF() + relativeLightness);

    mixedColor.setHslF(originalColor.hslHueF(), mixedSaturation, mixedLightness);

    return mixedColor;
}
}

uint qHash(const TextStyle &textStyle)
{
    return ::qHash(quint8(textStyle));
}
uint qHash(TextStyles textStyles)
{
    return ::qHash(reinterpret_cast<quint64&>(textStyles));
}

bool operator==(const TextStyles &first, const TextStyles &second)
{
    return first.mainStyle == second.mainStyle
        && first.mixinStyles == second.mixinStyles;
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

QTextCharFormat FontColorsSettings::toTextCharFormat(TextStyle category) const
{
    using namespace TextEditor;

    auto textCharFormatIterator = m_formatCache.find(category);
    if (textCharFormatIterator != m_formatCache.end())
        return *textCharFormatIterator;

    const Format &f = m_scheme.formatFor(category);
    QTextCharFormat tf;

    if (category == C_TEXT)
        tf.setFont(toFont());

    if (category == C_OCCURRENCES_UNUSED)
        tf.setToolTip(QObject::tr("Unused variable"));

    if (f.foreground().isValid()
            && category != C_OCCURRENCES
            && category != C_OCCURRENCES_RENAME
            && category != C_SEARCH_RESULT
            && category != C_PARENTHESES_MISMATCH)
        tf.setForeground(f.foreground());
    if (f.background().isValid() && (category == C_TEXT || f.background() != m_scheme.formatFor(C_TEXT).background()))
        tf.setBackground(f.background());

    // underline does not need to fill without having background color
    if (f.underlineStyle() != QTextCharFormat::NoUnderline && !f.background().isValid())
        tf.setBackground(QBrush(Qt::BrushStyle::NoBrush));

    tf.setFontWeight(f.bold() ? QFont::Bold : (fontPreferThick ? QFont::Medium : QFont::Normal));
    tf.setFontItalic(f.italic());

    tf.setUnderlineColor(f.underlineColor());
    tf.setUnderlineStyle(f.underlineStyle());

    m_formatCache.insert(category, tf);
    return tf;
}

QTextCharFormat FontColorsSettings::toTextCharFormat(TextStyles textStyles) const
{
    auto textCharFormatIterator = m_textCharFormatCache.find(textStyles);
    if (textCharFormatIterator != m_textCharFormatCache.end())
        return *textCharFormatIterator;

    QTextCharFormat textCharFormat = toTextCharFormat(textStyles.mainStyle);

    addMixinStyle(textCharFormat, textStyles.mixinStyles);

    m_textCharFormatCache.insert(textStyles, textCharFormat);

    return textCharFormat;
}

void FontColorsSettings::addMixinStyle(QTextCharFormat& textCharFormat, const MixinTextStyles& mixinStyles) const
{
    using namespace TextEditor;

    for (TextStyle mixinStyle : mixinStyles) {
        const Format &format = m_scheme.formatFor(mixinStyle);

        if (textCharFormat.hasProperty(QTextFormat::ForegroundBrush)) {
            if (format.foreground().isValid())
                textCharFormat.setForeground(format.foreground());
            else
                textCharFormat.setForeground(mixBrush(textCharFormat.foreground(),
                                                      format.relativeForegroundSaturation(),
                                                      format.relativeForegroundLightness()));
        }
        if (textCharFormat.hasProperty(QTextFormat::BackgroundBrush)) {
            if (format.background().isValid())
                textCharFormat.setBackground(format.background());
            else
                textCharFormat.setBackground(mixBrush(textCharFormat.background(),
                                                      format.relativeBackgroundSaturation(),
                                                      format.relativeBackgroundLightness()));
        }
        if (!textCharFormat.fontItalic())
            textCharFormat.setFontItalic(format.italic());

        if (textCharFormat.fontWeight() == QFont::Normal || textCharFormat.fontWeight() == QFont::Medium)
            textCharFormat.setFontWeight(format.bold() ? QFont::Bold : (fontPreferThick ? QFont::Medium : QFont::Normal));

        if (textCharFormat.underlineStyle() == QTextCharFormat::NoUnderline) {
            textCharFormat.setUnderlineStyle(format.underlineStyle());
            textCharFormat.setUnderlineColor(format.underlineColor());
        }
    }
}