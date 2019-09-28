#ifndef DESIGNERSETTINGS_H
#define DESIGNERSETTINGS_H

#include <groupsettings.h>

struct SceneSettings;
struct ToolboxSettings;

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
    static ToolboxSettings* toolboxSettings();
    const char* group() const override;

signals:
    void sceneSettingsChanged();
    void toolboxSettingsChanged();

private:
    explicit DesignerSettings(QObject* parent = nullptr);
    ~DesignerSettings() override;

private:
    static DesignerSettings* s_instance;
    static SceneSettings* s_sceneSettings;
    static ToolboxSettings* s_toolboxSettings;
};

#endif // DESIGNERSETTINGS_H