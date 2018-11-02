#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <groupsettings.h>

struct InterfaceSettings;

class GeneralSettings final : public GroupSettings
{
    Q_OBJECT
    friend class ApplicationCore; //  For constructor

public:
    static GeneralSettings* instance();
    static void read();
    static void write();
    static void reset();
    static InterfaceSettings* interfaceSettings();

    const char* group() const override;

signals:
    void interfaceSettingsChanged();
    void resetWindowSettings();

private:
    explicit GeneralSettings(QObject* parent = nullptr);
    ~GeneralSettings();

private:
    static GeneralSettings* s_instance;
    static InterfaceSettings* s_interfaceSettings;
};

#endif // GENERALSETTINGS_H