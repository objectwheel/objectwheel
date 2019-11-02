#ifndef CODEEDITORSETTINGS_H
#define CODEEDITORSETTINGS_H

#include <groupsettings.h>

struct BehaviorSettings;
struct FontColorsSettings;

class CodeEditorSettings final : public GroupSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditorSettings)

    friend class ApplicationCore; //  For constructor

public:
    static CodeEditorSettings* instance();
    static void read();
    static void write();
    static void reset();
    const char* group() const override;

    static BehaviorSettings* behaviorSettings();
    static FontColorsSettings* fontColorsSettings();

signals:
    void behaviorSettingsChanged();
    void fontColorsSettingsChanged();

private:
    explicit CodeEditorSettings(QObject* parent = nullptr);
    ~CodeEditorSettings() override;

private:
    static CodeEditorSettings* s_instance;
    static BehaviorSettings* s_behaviorSettings;
    static FontColorsSettings* s_fontColorsSettings;
};

#endif // CODEEDITORSETTINGS_H