#include "interfacesettingswidget.h"

InterfaceSettingsWidget::InterfaceSettingsWidget(QWidget *parent) : SettingsWidget(parent)
{
}

QString InterfaceSettingsWidget::title() const
{
    return tr("Interface");
}
