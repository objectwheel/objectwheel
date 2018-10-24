#ifndef BOOTSETTINGS_H
#define BOOTSETTINGS_H

#include <generalsettings.h>
#include <QSettings>

class BootSettings final
{
    Q_DISABLE_COPY(BootSettings)

public:
    static void init(const char* filePath);
    static QSettings* settings();
    static const char* resourcePath();
    static const char* userResourcePath();

private:
    BootSettings(const char* filePath);
    ~BootSettings();

private:
    static BootSettings* s_instance;
    QSettings m_settings;
    GeneralSettings m_generalSettings;
};

#endif // BOOTSETTINGS_H