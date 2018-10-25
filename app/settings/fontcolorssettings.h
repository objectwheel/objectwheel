#ifndef FONTCOLORSSETTINGS_H
#define FONTCOLORSSETTINGS_H

#include <settings.h>
#include <texteditor/textstyles.h>
#include <texteditor/colorscheme.h>

#include <QFont>
#include <QHash>
#include <QTextCharFormat>

class CodeEditorSettings;

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