#ifndef DESIGNERSETTINGS_H
#define DESIGNERSETTINGS_H

#include <groupsettings.h>

struct SceneSettings;
class DesignerSettings final : public GroupSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerSettings)

    friend class ApplicationCore; //  For constructor

public:
    static DesignerSettings* instance();
    static void read();
    static void write();
    static void reset();
    static SceneSettings* sceneSettings();
    const char* group() const override;

signals:
    void sceneSettingsChanged();

private:
    explicit DesignerSettings(QObject* parent = nullptr);
    ~DesignerSettings() override;

private:
    static DesignerSettings* s_instance;
    static SceneSettings* s_sceneSettings;
};

#endif // DESIGNERSETTINGS_H