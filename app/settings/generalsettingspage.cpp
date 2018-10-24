#include <generalsettingspage.h>
#include <interfacesettingswidget.h>
#include <QIcon>

GeneralSettingsPage::GeneralSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("General"));
    addWidget(new InterfaceSettingsWidget(this));
}

QIcon GeneralSettingsPage::icon() const
{
    return QIcon(":/images/settings/general.png");
}

bool GeneralSettingsPage::containsWord(const QString& /*word*/) const
{
    // TODO
}
