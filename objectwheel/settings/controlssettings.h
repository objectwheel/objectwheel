#ifndef CONTROLSSETTINGS_H
#define CONTROLSSETTINGS_H

#include <settings.h>

class DesignerSettings;
struct ControlsSettings final : public Settings
{
    explicit ControlsSettings(DesignerSettings* designerSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    int itemDoubleClickAction;
};

#endif // CONTROLSSETTINGS_H