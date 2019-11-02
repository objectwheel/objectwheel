#ifndef TOOLBOXSETTINGS_H
#define TOOLBOXSETTINGS_H

#include <settings.h>

class DesignerSettings;
struct ToolboxSettings final : public Settings
{
    explicit ToolboxSettings(DesignerSettings* designerSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool enableAlternatingRowColors;
    int textElideMode;
    int iconSize;
};

#endif // TOOLBOXSETTINGS_H