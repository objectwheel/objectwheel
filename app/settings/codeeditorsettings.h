#ifndef CODEEDITORSETTINGS_H
#define CODEEDITORSETTINGS_H

#include <groupsettings.h>

struct BehaviorSettings;
struct FontColorsSettings;

class CodeEditorSettings final : public GroupSettings
{
    Q_OBJECT
    friend class ApplicationCore; //  For constructor

public:
    static CodeEditorSettings* instance();
    static void read();
    static void write();
    static void reset();

    static BehaviorSettings* behaviorSettings();
    static FontColorsSettings* fontColorsSettings();

    const char* group() const override;

signals:
    void behaviorSettingsChanged();
    void fontColorsSettingsChanged();

private:
    explicit CodeEditorSettings(QObject* parent = nullptr);
    ~CodeEditorSettings();

private:
    static CodeEditorSettings* s_instance;
    static BehaviorSettings* s_behaviorSettings;
    static FontColorsSettings* s_fontColorsSettings;
};

#endif // CODEEDITORSETTINGS_H