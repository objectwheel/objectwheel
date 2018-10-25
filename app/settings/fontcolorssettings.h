#ifndef FONTCOLORSSETTINGS_H
#define FONTCOLORSSETTINGS_H

#include <settings.h>
#include <texteditor/textstyles.h>

#include <QFont>
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
    QTextCharFormat toTextCharFormat(TextEditor::TextStyle category) const;
    QTextCharFormat toTextCharFormat(TextEditor::TextStyles textStyles) const;

    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
};

#endif // FONTCOLORSSETTINGS_H