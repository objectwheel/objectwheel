#include <fontcolorssettings.h>
#include <codeeditorsettings.h>
#include <applicationcore.h>

#include <QPalette>
#include <QDebug>
#include <QFileInfo>

namespace {

const char* g_fontFamily = "FontFamily";
const char* g_fontPixelSize = "FontPixelSize";
const char* g_fontPreferThick = "FontPreferThick";
const char* g_fontPreferAntialiasing = "FontPreferAntialiasing";
const char* g_colorSchemeFileName = "ColorSchemeFileName";

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

FormatDescriptions defaultFormatDescriptions()
{
    using namespace TextEditor;
    // Note: default background colors are coming from FormatDescription::background()
    // Add font preference page
    static const FormatDescriptions& formatDescr = [] () -> FormatDescriptions {
        static FormatDescriptions description;
        description.reserve(C_LAST_STYLE_SENTINEL);
        description.emplace_back(C_TEXT, QObject::tr("Text"), QObject::tr("Generic text.\nApplied to "
                                                                          "text, if no other "
                                                                          "rules matching."));

        // Special categories
        description.emplace_back(C_LINK, QObject::tr("Link"),
                                 QObject::tr("Links that follow symbol under cursor."), Qt::blue);
        description.emplace_back(C_SELECTION, QObject::tr("Selection"), QObject::tr("Selected text."),
                                 ApplicationCore::palette().highlightedText().color());
        description.emplace_back(C_LINE_NUMBER, QObject::tr("Line Number"),
                                 QObject::tr("Line numbers located on the left side of the editor."),
                                 FormatDescription::AllControlsExceptUnderline);
        description.emplace_back(C_SEARCH_RESULT, QObject::tr("Search Result"),
                                 QObject::tr("Highlighted search results inside the editor."),
                                 FormatDescription::ShowBackgroundControl);
        description.emplace_back(C_SEARCH_SCOPE, QObject::tr("Search Scope"),
                                 QObject::tr("Section where the pattern is searched in."),
                                 FormatDescription::ShowBackgroundControl);
        description.emplace_back(C_PARENTHESES, QObject::tr("Parentheses"),
                                 QObject::tr("Displayed when matching parentheses, square brackets "
                                             "or curly brackets are found."));
        description.emplace_back(C_PARENTHESES_MISMATCH, QObject::tr("Mismatched Parentheses"),
                                 QObject::tr("Displayed when mismatched parentheses, "
                                             "square brackets, or curly brackets are found."));
        description.emplace_back(C_AUTOCOMPLETE, QObject::tr("Auto Complete"),
                                 QObject::tr("Displayed when a character is automatically inserted "
                                             "like brackets or quotes."));
        description.emplace_back(C_CURRENT_LINE, QObject::tr("Current Line"),
                                 QObject::tr("Line where the cursor is placed in."),
                                 FormatDescription::ShowBackgroundControl);

        FormatDescription currentLineNumber(C_CURRENT_LINE_NUMBER,
                                            QObject::tr("Current Line Number"),
                                            QObject::tr("Line number located on the left side of the "
                                                        "editor where the cursor is placed in."),
                                            Qt::darkGray,
                                            FormatDescription::AllControlsExceptUnderline);
        currentLineNumber.format().setBold(true);
        description.push_back(std::move(currentLineNumber));


        description.emplace_back(C_OCCURRENCES, QObject::tr("Occurrences"),
                                 QObject::tr("Occurrences of the symbol under the cursor.\n"
                                             "(Only the background will be applied.)"),
                                 FormatDescription::ShowBackgroundControl);
        description.emplace_back(C_OCCURRENCES_UNUSED,
                                 QObject::tr("Unused Occurrence"),
                                 QObject::tr("Occurrences of unused variables."),
                                 Qt::darkYellow,
                                 QTextCharFormat::SingleUnderline);
        description.emplace_back(C_OCCURRENCES_RENAME, QObject::tr("Renaming Occurrence"),
                                 QObject::tr("Occurrences of a symbol that will be renamed."),
                                 FormatDescription::ShowBackgroundControl);

        // Standard categories
        description.emplace_back(C_NUMBER, QObject::tr("Number"), QObject::tr("Number literal."),
                                 Qt::darkBlue);
        description.emplace_back(C_STRING, QObject::tr("String"),
                                 QObject::tr("Character and string literals."), Qt::darkGreen);
        description.emplace_back(C_PRIMITIVE_TYPE, QObject::tr("Primitive Type"),
                                 QObject::tr("Name of a primitive data type."), Qt::darkYellow);
        description.emplace_back(C_TYPE, QObject::tr("Type"), QObject::tr("Name of a type."),
                                 Qt::darkMagenta);
        description.emplace_back(C_LOCAL, QObject::tr("Local"),
                                 QObject::tr("Local variables."), QColor(9, 46, 100));
        description.emplace_back(C_FIELD, QObject::tr("Field"),
                                 QObject::tr("Class' data members."), Qt::darkRed);
        description.emplace_back(C_GLOBAL, QObject::tr("Global"),
                                 QObject::tr("Global variables."), QColor(206, 92, 0));
        description.emplace_back(C_ENUMERATION, QObject::tr("Enumeration"),
                                 QObject::tr("Applied to enumeration items."), Qt::darkMagenta);

        Format functionFormat;
        functionFormat.setForeground(QColor(0, 103, 124));
        description.emplace_back(C_FUNCTION, QObject::tr("Function"), QObject::tr("Name of a function."),
                                 functionFormat);
        functionFormat.setItalic(true);
        description.emplace_back(C_VIRTUAL_METHOD, QObject::tr("Virtual Function"),
                                 QObject::tr("Name of function declared as virtual."),
                                 functionFormat);

        description.emplace_back(C_BINDING, QObject::tr("QML Binding"),
                                 QObject::tr("QML item property, that allows a "
                                             "binding to another property."),
                                 Qt::darkRed);

        Format qmlLocalNameFormat;
        qmlLocalNameFormat.setItalic(true);
        description.emplace_back(C_QML_LOCAL_ID, QObject::tr("QML Local Id"),
                                 QObject::tr("QML item id within a QML file."), qmlLocalNameFormat);
        description.emplace_back(C_QML_ROOT_OBJECT_PROPERTY,
                                 QObject::tr("QML Root Object Property"),
                                 QObject::tr("QML property of a parent item."), qmlLocalNameFormat);
        description.emplace_back(C_QML_SCOPE_OBJECT_PROPERTY,
                                 QObject::tr("QML Scope Object Property"),
                                 QObject::tr("Property of the same QML item."), qmlLocalNameFormat);
        description.emplace_back(C_QML_STATE_NAME, QObject::tr("QML State Name"),
                                 QObject::tr("Name of a QML state."), qmlLocalNameFormat);

        description.emplace_back(C_QML_TYPE_ID, QObject::tr("QML Type Name"),
                                 QObject::tr("Name of a QML type."), Qt::darkMagenta);

        Format qmlExternalNameFormat = qmlLocalNameFormat;
        qmlExternalNameFormat.setForeground(Qt::darkBlue);
        description.emplace_back(C_QML_EXTERNAL_ID, QObject::tr("QML External Id"),
                                 QObject::tr("QML id defined in another QML file."),
                                 qmlExternalNameFormat);
        description.emplace_back(C_QML_EXTERNAL_OBJECT_PROPERTY,
                                 QObject::tr("QML External Object Property"),
                                 QObject::tr("QML property defined in another QML file."),
                                 qmlExternalNameFormat);

        Format jsLocalFormat;
        jsLocalFormat.setForeground(QColor(41, 133, 199)); // very light blue
        jsLocalFormat.setItalic(true);
        description.emplace_back(C_JS_SCOPE_VAR, QObject::tr("JavaScript Scope Var"),
                                 QObject::tr("Variables defined inside the JavaScript file."),
                                 jsLocalFormat);

        Format jsGlobalFormat;
        jsGlobalFormat.setForeground(QColor(0, 85, 175)); // light blue
        jsGlobalFormat.setItalic(true);
        description.emplace_back(C_JS_IMPORT_VAR, QObject::tr("JavaScript Import"),
                                 QObject::tr("Name of a JavaScript import inside a QML file."),
                                 jsGlobalFormat);
        description.emplace_back(C_JS_GLOBAL_VAR, QObject::tr("JavaScript Global Variable"),
                                 QObject::tr("Variables defined outside the script."),
                                 jsGlobalFormat);

        description.emplace_back(C_KEYWORD, QObject::tr("Keyword"),
                                 QObject::tr("Reserved keywords of the programming language except "
                                             "keywords denoting primitive types."), Qt::darkYellow);
        description.emplace_back(C_OPERATOR, QObject::tr("Operator"),
                                 QObject::tr("Operators (for example operator++ or operator-=)."));
        description.emplace_back(C_PREPROCESSOR, QObject::tr("Preprocessor"),
                                 QObject::tr("Preprocessor directives."), Qt::darkBlue);
        description.emplace_back(C_LABEL, QObject::tr("Label"), QObject::tr("Labels for goto statements."),
                                 Qt::darkRed);
        description.emplace_back(C_COMMENT, QObject::tr("Comment"),
                                 QObject::tr("All style of comments except Doxygen comments."),
                                 Qt::darkGreen);
        description.emplace_back(C_DOXYGEN_COMMENT, QObject::tr("Doxygen Comment"),
                                 QObject::tr("Doxygen comments."), Qt::darkBlue);
        description.emplace_back(C_DOXYGEN_TAG, QObject::tr("Doxygen Tag"), QObject::tr("Doxygen tags."),
                                 Qt::blue);
        description.emplace_back(C_VISUAL_WHITESPACE, QObject::tr("Visual Whitespace"),
                                 QObject::tr("Whitespace.\nWill not be applied to whitespace "
                                             "in comments and strings."), Qt::lightGray);
        description.emplace_back(C_DISABLED_CODE, QObject::tr("Disabled Code"),
                                 QObject::tr("Code disabled by preprocessor directives."));

        // Diff categories
        description.emplace_back(C_ADDED_LINE, QObject::tr("Added Line"),
                                 QObject::tr("Applied to added lines in differences (in diff editor)."),
                                 QColor(0, 170, 0));
        description.emplace_back(C_REMOVED_LINE, QObject::tr("Removed Line"),
                                 QObject::tr("Applied to removed lines in differences (in diff editor)."),
                                 Qt::red);
        description.emplace_back(C_DIFF_FILE, QObject::tr("Diff File"),
                                 QObject::tr("Compared files (in diff editor)."), Qt::darkBlue);
        description.emplace_back(C_DIFF_LOCATION, QObject::tr("Diff Location"),
                                 QObject::tr("Location in the files where the difference is "
                                             "(in diff editor)."), Qt::blue);

        // New diff categories
        description.emplace_back(C_DIFF_FILE_LINE, QObject::tr("Diff File Line"),
                                 QObject::tr("Applied to lines with file information "
                                             "in differences (in side-by-side diff editor)."),
                                 Format(QColor(), QColor(255, 255, 0)));
        description.emplace_back(C_DIFF_CONTEXT_LINE, QObject::tr("Diff Context Line"),
                                 QObject::tr("Applied to lines describing hidden context "
                                             "in differences (in side-by-side diff editor)."),
                                 Format(QColor(), QColor(175, 215, 231)));
        description.emplace_back(C_DIFF_SOURCE_LINE, QObject::tr("Diff Source Line"),
                                 QObject::tr("Applied to source lines with changes "
                                             "in differences (in side-by-side diff editor)."),
                                 Format(QColor(), QColor(255, 223, 223)));
        description.emplace_back(C_DIFF_SOURCE_CHAR, QObject::tr("Diff Source Character"),
                                 QObject::tr("Applied to removed characters "
                                             "in differences (in side-by-side diff editor)."),
                                 Format(QColor(), QColor(255, 175, 175)));
        description.emplace_back(C_DIFF_DEST_LINE, QObject::tr("Diff Destination Line"),
                                 QObject::tr("Applied to destination lines with changes "
                                             "in differences (in side-by-side diff editor)."),
                                 Format(QColor(), QColor(223, 255, 223)));
        description.emplace_back(C_DIFF_DEST_CHAR, QObject::tr("Diff Destination Character"),
                                 QObject::tr("Applied to added characters "
                                             "in differences (in side-by-side diff editor)."),
                                 Format(QColor(), QColor(175, 255, 175)));

        description.emplace_back(C_LOG_CHANGE_LINE, QObject::tr("Log Change Line"),
                                 QObject::tr("Applied to lines describing changes in VCS log."),
                                 Format(QColor(192, 0, 0), QColor()));

        // Mixin categories
        description.emplace_back(C_ERROR,
                                 QObject::tr("Error"),
                                 QObject::tr("Underline color of error diagnostics."),
                                 QColor(255,0, 0),
                                 QTextCharFormat::SingleUnderline,
                                 FormatDescription::ShowUnderlineControl);
        description.emplace_back(C_ERROR_CONTEXT,
                                 QObject::tr("Error Context"),
                                 QObject::tr("Underline color of the contexts of error diagnostics."),
                                 QColor(255,0, 0),
                                 QTextCharFormat::DotLine,
                                 FormatDescription::ShowUnderlineControl);
        description.emplace_back(C_WARNING,
                                 QObject::tr("Warning"),
                                 QObject::tr("Underline color of warning diagnostics."),
                                 QColor(255, 190, 0),
                                 QTextCharFormat::SingleUnderline,
                                 FormatDescription::ShowUnderlineControl);
        description.emplace_back(C_WARNING_CONTEXT,
                                 QObject::tr("Warning Context"),
                                 QObject::tr("Underline color of the contexts of warning diagnostics."),
                                 QColor(255, 190, 0),
                                 QTextCharFormat::DotLine,
                                 FormatDescription::ShowUnderlineControl);
        Format declarationFormat = Format::createMixinFormat();
        declarationFormat.setBold(true);
        description.emplace_back(C_DECLARATION,
                                 QObject::tr("Function Declaration"),
                                 QObject::tr("Style adjustments to (function) declarations."),
                                 declarationFormat,
                                 FormatDescription::ShowFontUnderlineAndRelativeControls);
        description.emplace_back(C_FUNCTION_DEFINITION,
                                 QObject::tr("Function Definition"),
                                 QObject::tr("Name of function at its definition."),
                                 Format::createMixinFormat());
        Format outputArgumentFormat = Format::createMixinFormat();
        outputArgumentFormat.setItalic(true);
        description.emplace_back(C_OUTPUT_ARGUMENT,
                                 QObject::tr("Output Argument"),
                                 QObject::tr("Writable arguments of a function call."),
                                 outputArgumentFormat,
                                 FormatDescription::ShowFontUnderlineAndRelativeControls);
        return description;
    }();
    return formatDescr;
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

FormatDescription::FormatDescription(TextStyle id,
                                     const QString &displayName,
                                     const QString &tooltipText,
                                     const QColor &foreground,
                                     FormatDescription::ShowControls showControls)
    : m_id(id),
      m_displayName(displayName),
      m_tooltipText(tooltipText),
      m_showControls(showControls)
{
    m_format.setForeground(foreground);
    m_format.setBackground(defaultBackground(id));
}

FormatDescription::FormatDescription(TextStyle id,
                                     const QString &displayName,
                                     const QString &tooltipText,
                                     const TextEditor::Format &format,
                                     FormatDescription::ShowControls showControls)
    : m_id(id),
      m_format(format),
      m_displayName(displayName),
      m_tooltipText(tooltipText),
      m_showControls(showControls)
{
}

FormatDescription::FormatDescription(TextStyle id,
                                     const QString &displayName,
                                     const QString &tooltipText,
                                     const QColor &underlineColor,
                                     const QTextCharFormat::UnderlineStyle underlineStyle,
                                     FormatDescription::ShowControls showControls)
    : m_id(id),
      m_displayName(displayName),
      m_tooltipText(tooltipText),
      m_showControls(showControls)
{
    m_format.setForeground(defaultForeground(id));
    m_format.setBackground(defaultBackground(id));
    m_format.setUnderlineColor(underlineColor);
    m_format.setUnderlineStyle(underlineStyle);
}

FormatDescription::FormatDescription(TextStyle id,
                                     const QString &displayName,
                                     const QString &tooltipText,
                                     FormatDescription::ShowControls showControls)
    : m_id(id),
      m_displayName(displayName),
      m_tooltipText(tooltipText),
      m_showControls(showControls)
{
    m_format.setForeground(defaultForeground(id));
    m_format.setBackground(defaultBackground(id));
}

QColor FormatDescription::defaultForeground(TextStyle id)
{
    if (id == C_LINE_NUMBER) {
        const QPalette palette = ApplicationCore::palette();
        const QColor bg = palette.background().color();
        if (bg.value() < 128)
            return palette.foreground().color();
        else
            return palette.dark().color();
    } else if (id == C_CURRENT_LINE_NUMBER) {
        const QPalette palette = ApplicationCore::palette();
        const QColor bg = palette.background().color();
        if (bg.value() < 128)
            return palette.foreground().color();
        else
            return QColor();
    } else if (id == C_PARENTHESES) {
        return QColor(Qt::red);
    } else if (id == C_AUTOCOMPLETE) {
        return QColor(Qt::darkBlue);
    }
    return QColor();
}

QColor FormatDescription::defaultBackground(TextStyle id)
{
    if (id == C_TEXT) {
        return Qt::white;
    } else if (id == C_LINE_NUMBER) {
        return ApplicationCore::palette().background().color();
    } else if (id == C_SEARCH_RESULT) {
        return QColor(0xffef0b);
    } else if (id == C_PARENTHESES) {
        return QColor(0xb4, 0xee, 0xb4);
    } else if (id == C_PARENTHESES_MISMATCH) {
        return QColor(Qt::magenta);
    } else if (id == C_AUTOCOMPLETE) {
        return QColor(192, 192, 255);
    } else if (id == C_CURRENT_LINE || id == C_SEARCH_SCOPE) {
        const QPalette palette = ApplicationCore::palette();
        const QColor &fg = palette.color(QPalette::Highlight);
        const QColor &bg = palette.color(QPalette::Base);

        qreal smallRatio;
        qreal largeRatio;
        if (id == C_CURRENT_LINE) {
            smallRatio = .3;
            largeRatio = .6;
        } else {
            smallRatio = .05;
            largeRatio = .4;
        }
        const qreal ratio = ((palette.color(QPalette::Text).value() < 128)
                             ^ (palette.color(QPalette::HighlightedText).value() < 128)) ? smallRatio : largeRatio;

        const QColor &col = QColor::fromRgbF(fg.redF() * ratio + bg.redF() * (1 - ratio),
                                             fg.greenF() * ratio + bg.greenF() * (1 - ratio),
                                             fg.blueF() * ratio + bg.blueF() * (1 - ratio));
        return col;
    } else if (id == C_SELECTION) {
        return ApplicationCore::palette().color(QPalette::Highlight);
    } else if (id == C_OCCURRENCES) {
        return QColor(180, 180, 180);
    } else if (id == C_OCCURRENCES_RENAME) {
        return QColor(255, 100, 100);
    } else if (id == C_DISABLED_CODE) {
        return QColor(239, 239, 239);
    }
    return QColor(); // invalid color
}

bool FormatDescription::showControl(FormatDescription::ShowControls showControl) const
{
    return m_showControls & showControl;
}

FontColorsSettings::FontColorsSettings(CodeEditorSettings* codeEditorSettings) : Settings(codeEditorSettings)
  , defaultFormatDescriptions(::defaultFormatDescriptions())
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
    colorSchemeFileName = value<QString>(g_colorSchemeFileName, colorSchemeFileName);
    loadColorScheme(colorSchemeFileName);
    end();
}

void FontColorsSettings::write()
{
    begin();
    setValue(g_fontPreferThick, fontPreferThick);
    setValue(g_fontPreferAntialiasing, fontPreferAntialiasing);
    setValue(g_fontPixelSize, fontPixelSize);
    setValue(g_fontFamily, fontFamily);
    setValue(g_colorSchemeFileName, colorSchemeFileName);
    if (QFileInfo(colorSchemeFileName).isWritable())
        colorScheme.save(colorSchemeFileName, nullptr);
    end();

    emit static_cast<CodeEditorSettings*>(groupSettings())->fontColorsSettingsChanged();
}

void FontColorsSettings::reset()
{
    fontPreferThick = false;
    fontPreferAntialiasing = true;
    fontPixelSize = 14;
    fontFamily = "Inconsolata";
    loadColorScheme(":/styles/default.xml");
}

const char* FontColorsSettings::category() const
{
    return "FontColors";
}

QFont FontColorsSettings::toFont() const
{
    QFont font(fontFamily);
    font.setPixelSize(fontPixelSize);
    font.setWeight(fontPreferThick ? QFont::Bold : QFont::Normal);
    font.setStyleStrategy(fontPreferAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    return font;
}

QTextCharFormat FontColorsSettings::toTextCharFormat(TextStyle category) const
{
    using namespace TextEditor;

    auto textCharFormatIterator = m_formatCache.find(category);
    if (textCharFormatIterator != m_formatCache.end())
        return *textCharFormatIterator;

    const TextEditor::Format &f = colorScheme.formatFor(category);
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
    if (f.background().isValid() && (category == C_TEXT || f.background() != colorScheme.formatFor(C_TEXT).background()))
        tf.setBackground(f.background());

    // underline does not need to fill without having background color
    if (f.underlineStyle() != QTextCharFormat::NoUnderline && !f.background().isValid())
        tf.setBackground(QBrush(Qt::BrushStyle::NoBrush));

    tf.setFontWeight((f.bold() || fontPreferThick) ? QFont::Bold : QFont::Normal);
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

bool FontColorsSettings::loadColorScheme(const QString& fileName)
{
    bool loaded = true;
    colorSchemeFileName = fileName;

    m_formatCache.clear();
    m_textCharFormatCache.clear();

    if (!colorScheme.load(colorSchemeFileName)) {
        loaded = false;
        colorSchemeFileName.clear();
        qWarning() << "Failed to load color scheme:" << fileName;
    }

    // Apply default formats to undefined categories
    for (const FormatDescription &desc : defaultFormatDescriptions) {
        const TextStyle id = desc.id();
        if (!colorScheme.contains(id)) {
            TextEditor::Format format;
            const TextEditor::Format &descFormat = desc.format();
            if (descFormat == format && colorScheme.contains(C_TEXT)) {
                // Default format -> Text
                const TextEditor::Format textFormat = colorScheme.formatFor(C_TEXT);
                format.setForeground(textFormat.foreground());
                format.setBackground(textFormat.background());
            } else {
                format.setForeground(descFormat.foreground());
                format.setBackground(descFormat.background());
            }
            format.setRelativeForegroundSaturation(descFormat.relativeForegroundSaturation());
            format.setRelativeForegroundLightness(descFormat.relativeForegroundLightness());
            format.setRelativeBackgroundSaturation(descFormat.relativeBackgroundSaturation());
            format.setRelativeBackgroundLightness(descFormat.relativeBackgroundLightness());
            format.setBold(descFormat.bold());
            format.setItalic(descFormat.italic());
            format.setUnderlineColor(descFormat.underlineColor());
            format.setUnderlineStyle(descFormat.underlineStyle());
            colorScheme.setFormatFor(id, format);
        }
    }

    return loaded;
}

bool FontColorsSettings::saveColorScheme(const QString& fileName)
{
    const bool saved = colorScheme.save(fileName, nullptr);
    if (saved)
        colorSchemeFileName = fileName;
    return saved;
}

void FontColorsSettings::addMixinStyle(QTextCharFormat& textCharFormat, const MixinTextStyles& mixinStyles) const
{
    using namespace TextEditor;

    for (TextStyle mixinStyle : mixinStyles) {
        const TextEditor::Format &format = colorScheme.formatFor(mixinStyle);
        
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
        
        if (textCharFormat.fontWeight() == QFont::Normal)
            textCharFormat.setFontWeight((format.bold() || fontPreferThick) ? QFont::Bold : QFont::Normal);
        
        if (textCharFormat.underlineStyle() == QTextCharFormat::NoUnderline) {
            textCharFormat.setUnderlineStyle(format.underlineStyle());
            textCharFormat.setUnderlineColor(format.underlineColor());
        }
    }
}