#include <systemsettingspage.h>
#include <updatesettingswidget.h>
#include <QIcon>

SystemSettingsPage::SystemSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("System"));
    addWidget(new UpdateSettingsWidget(this));
}

QIcon SystemSettingsPage::icon() const
{
    return QIcon(":/images/settings/system.svg");
}
