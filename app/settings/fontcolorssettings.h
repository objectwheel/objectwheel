#ifndef FONTCOLORSSETTINGS_H
#define FONTCOLORSSETTINGS_H

#include <settings.h>

class CodeEditorSettings;

struct FontColorsSettings : public Settings
{
    FontColorsSettings(CodeEditorSettings* codeEditorSettings = nullptr);

    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
};

#endif // FONTCOLORSSETTINGS_H