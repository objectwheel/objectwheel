#ifndef NAVIGATORSETTINGS_H
#define NAVIGATORSETTINGS_H

#include <settings.h>

class DesignerSettings;
struct NavigatorSettings final : public Settings
{
    explicit NavigatorSettings(DesignerSettings* designerSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    int itemDoubleClickAction;
};

#endif // NAVIGATORSETTINGS_H