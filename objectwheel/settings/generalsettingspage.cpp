#include <generalsettingspage.h>
#include <interfacesettingswidget.h>
#include <updatesettingswidget.h>
#include <QIcon>

GeneralSettingsPage::GeneralSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("General"));
    addWidget(new InterfaceSettingsWidget(this));
    addWidget(new UpdateSettingsWidget(this));
}

QIcon GeneralSettingsPage::icon() const
{
    return QIcon(":/images/settings/general.svg");
}
