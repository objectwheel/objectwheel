#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include <settings.h>
#include <QColor>

struct InterfaceSettings : Settings
{
    InterfaceSettings(const QString& group);

    void read() override;
    void write() override;
    void reset() override;

    QColor color;
    QString theme;
    QString language;
    bool hdpiEnabled;
};

#endif // INTERFACESETTINGS_H