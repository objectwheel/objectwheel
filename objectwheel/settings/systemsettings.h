#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include <groupsettings.h>

struct UpdateSettings;

class SystemSettings final : public GroupSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemSettings)

    friend class ApplicationCore; //  For constructor

public:
    static SystemSettings* instance();
    static void read();
    static void write();
    static void reset();
    static UpdateSettings* updateSettings();
    const char* group() const override;

signals:
    void updateSettingsChanged();
    void designerStateReset();

private:
    explicit SystemSettings(QObject* parent = nullptr);
    ~SystemSettings() override;

private:
    static SystemSettings* s_instance;
    static UpdateSettings* s_updateSettings;
};

#endif // SYSTEMSETTINGS_H