#ifndef CODEEDITORSETTINGS_H
#define CODEEDITORSETTINGS_H

#include <groupsettings.h>

struct FontColorsSettings;

class CodeEditorSettings final : public GroupSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditorSettings)

    friend class ApplicationCore; //  Make it constructable only from ApplicationCore

public:
    static CodeEditorSettings* instance();
    static void read();
    static void write();
    static void reset();
    static FontColorsSettings* fontColorsSettings();

    const char* group() const override;

signals:
    void fontColorsSettingsChanged();

private:
    explicit CodeEditorSettings(QObject* parent = nullptr);
    ~CodeEditorSettings();

private:
    static CodeEditorSettings* s_instance;
    static FontColorsSettings* s_fontColorsSettings;
};

#endif // CODEEDITORSETTINGS_H