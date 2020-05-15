#ifndef UPDATESETTINGS_H
#define UPDATESETTINGS_H

#include <settings.h>
#include <QFont>
#include <QColor>

class GeneralSettings;
struct UpdateSettings final : public Settings
{
    explicit UpdateSettings(GeneralSettings* generalSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;
    QFont toFont() const;

    bool hdpiEnabled;
    int theme;
    int language;
    QColor highlightColor;
    /****/
    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
    /****/
    bool outputPanePops;
    bool preserveDesignerState;
    bool outputPaneMinimizedStartupEnabled;

    /****/
    bool checkForUpdatesAutomatically;
};

#endif // UPDATESETTINGS_H