#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QObject>

class InterfaceSettings;

class GeneralSettings final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GeneralSettings)

    friend class ApplicationCore; //  Make it constructable only from ApplicationCore

public:
    static GeneralSettings* instance();
    static InterfaceSettings* interfaceSettings();
    static void read();
    static void write();
    static void reset();

private:
    explicit GeneralSettings(QObject* parent = nullptr);
    ~GeneralSettings();

private:
    static GeneralSettings* s_instance;
    static InterfaceSettings* s_interfaceSettings;
};

#endif // GENERALSETTINGS_H