#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <settings.h>

struct InterfaceSettings;

class GeneralSettings
{
    friend class ApplicationCore; //  Make it constructable only from ApplicationCore
    Q_DISABLE_COPY(GeneralSettings)

public:
    static GeneralSettings* instance();
    static InterfaceSettings* interfaceSettings();
    static void read();
    static void write();
    static void reset();

private:
    GeneralSettings();
    ~GeneralSettings();

private:
    static GeneralSettings* s_instance;
    static InterfaceSettings* s_interfaceSettings;
};

#endif // GENERALSETTINGS_H