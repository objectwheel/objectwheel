#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <groupsettings.h>

struct InterfaceSettings;
struct UpdateSettings;

class GeneralSettings final : public GroupSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(GeneralSettings)

    friend class ApplicationCore; //  For constructor

public:
    static GeneralSettings* instance();
    static void read();
    static void write();
    static void reset();
    static InterfaceSettings* interfaceSettings();
    static UpdateSettings* updateSettings();
    const char* group() const override;

signals:
    void interfaceSettingsChanged();
    void updateSettingsChanged();
    void designerStateReset();

private:
    explicit GeneralSettings(QObject* parent = nullptr);
    ~GeneralSettings() override;

private:
    static GeneralSettings* s_instance;
    static InterfaceSettings* s_interfaceSettings;
    static UpdateSettings* s_updateSettings;
};

#endif // GENERALSETTINGS_H