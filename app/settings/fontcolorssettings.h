#ifndef FONTCOLORSSETTINGS_H
#define FONTCOLORSSETTINGS_H

#include <settings.h>
#include <texteditor/textstyles.h>
#include <texteditor/colorscheme.h>

#include <QFont>
#include <QHash>
#include <QTextCharFormat>

class CodeEditorSettings;

class FormatDescription
{
public:
    enum ShowControls {
        ShowForegroundControl = 0x1,
        ShowBackgroundControl = 0x2,
        ShowFontControls = 0x4,
        ShowUnderlineControl = 0x8,
        ShowRelativeForegroundControl = 0x10,
        ShowRelativeBackgroundControl = 0x20,
        ShowFontUnderlineAndRelativeControls = ShowFontControls
                                             | ShowUnderlineControl
                                             | ShowRelativeForegroundControl
                                             | ShowRelativeBackgroundControl,
        AllControls = 0xF,
        AllControlsExceptUnderline = AllControls & ~ShowUnderlineControl,
    };
    FormatDescription() = default;

    FormatDescription(TextStyle id,
                      const QString &displayName,
                      const QString &tooltipText,
                      ShowControls showControls = AllControls);

    FormatDescription(TextStyle id,
                      const QString &displayName,
                      const QString &tooltipText,
                      const QColor &foreground,
                      ShowControls showControls = AllControls);
    FormatDescription(TextStyle id,
                      const QString &displayName,
                      const QString &tooltipText,
                      const TextEditor::Format &format,
                      ShowControls showControls = AllControls);
    FormatDescription(TextStyle id,
                      const QString &displayName,
                      const QString &tooltipText,
                      const QColor &underlineColor,
                      const QTextCharFormat::UnderlineStyle underlineStyle,
                      ShowControls showControls = AllControls);

    TextStyle id() const { return m_id; }

    QString displayName() const
    { return m_displayName; }

    static QColor defaultForeground(TextStyle id);
    static QColor defaultBackground(TextStyle id);

    const TextEditor::Format &format() const { return m_format; }
    TextEditor::Format &format() { return m_format; }

    QString tooltipText() const
    { return  m_tooltipText; }

    bool showControl(ShowControls showControl) const;

private:
    TextStyle m_id;            // Name of the category
    TextEditor::Format m_format;            // Default format
    QString m_displayName;      // Displayed name of the category
    QString m_tooltipText;      // Description text for category
    ShowControls m_showControls = AllControls;
};

typedef std::vector<FormatDescription> FormatDescriptions;

struct FontColorsSettings : public Settings
{
    FontColorsSettings(CodeEditorSettings* codeEditorSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    QFont toFont() const;
    QTextCharFormat toTextCharFormat(TextStyle category) const;
    QTextCharFormat toTextCharFormat(TextStyles textStyles) const;

    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;

    TextEditor::ColorScheme m_scheme;
    mutable QHash<TextStyle, QTextCharFormat> m_formatCache;
    mutable QHash<TextStyles, QTextCharFormat> m_textCharFormatCache;

private:
    void addMixinStyle(QTextCharFormat &textCharFormat, const MixinTextStyles &mixinStyles) const;
};

#endif // FONTCOLORSSETTINGS_H