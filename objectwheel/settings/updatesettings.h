#ifndef UPDATESETTINGS_H
#define UPDATESETTINGS_H

#include <settings.h>
#include <QDateTime>

class SystemSettings;
struct UpdateSettings final : public Settings
{
    explicit UpdateSettings(SystemSettings* systemSettings);
    void read() override;
    void write() override;
    void reset() override;
    const char* category() const override;

    bool checkForUpdatesAutomatically;
    QDateTime lastUpdateCheckDate;
};

#endif // UPDATESETTINGS_H