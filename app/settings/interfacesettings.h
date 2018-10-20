#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include <settings.h>
#include <QColor>

class InterfaceSettings : public Settings
{
    Q_OBJECT

public:
    explicit InterfaceSettings(QObject* parent = nullptr);
    explicit InterfaceSettings(const QString& group, QObject* parent = nullptr);

    void read() override;
    void write() override;
    void reset() override;

    bool hdpiEnabled;
    QString theme;
    QString language;
    QColor topBarColor;
    QColor leftBarColor;
};

#endif // INTERFACESETTINGS_H