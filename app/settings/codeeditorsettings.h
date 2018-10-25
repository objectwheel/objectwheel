#ifndef CODEEDITORSETTINGS_H
#define CODEEDITORSETTINGS_H

#include <QObject>

class FontSettings;

class CodeEditorSettings final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditorSettings)

    friend class ApplicationCore; //  Make it constructable only from ApplicationCore

public:
    static CodeEditorSettings* instance();
    static FontSettings* fontSettings();
    static void read();
    static void write();
    static void reset();

private:
    explicit CodeEditorSettings(QObject* parent = nullptr);
    ~CodeEditorSettings();

private:
    static CodeEditorSettings* s_instance;
    static FontSettings* s_fontSettings;
};

#endif // CODEEDITORSETTINGS_H