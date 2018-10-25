#ifndef CODEEDITORSETTINGS_H
#define CODEEDITORSETTINGS_H

#include <QObject>

class FontColorsSettings;

class CodeEditorSettings final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditorSettings)

    friend class ApplicationCore; //  Make it constructable only from ApplicationCore

public:
    static CodeEditorSettings* instance();
    static FontColorsSettings* fontColorsSettings();
    static void read();
    static void write();
    static void reset();

private:
    explicit CodeEditorSettings(QObject* parent = nullptr);
    ~CodeEditorSettings();

private:
    static CodeEditorSettings* s_instance;
    static FontColorsSettings* s_fontColorsSettings;
};

#endif // CODEEDITORSETTINGS_H