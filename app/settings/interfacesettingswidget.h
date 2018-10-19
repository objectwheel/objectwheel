#ifndef INTERFACESETTINGSWIDGET_H
#define INTERFACESETTINGSWIDGET_H

#include <settingswidget.h>

class InterfaceSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit InterfaceSettingsWidget(QWidget* parent = nullptr);

    QString title() const override;

};

#endif // INTERFACESETTINGSWIDGET_H
