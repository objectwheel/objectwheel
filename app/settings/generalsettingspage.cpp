#include <generalsettingspage.h>
#include <interfacesettingswidget.h>
#include <QIcon>

GeneralSettingsPage::GeneralSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    addWidget(new InterfaceSettingsWidget(this));
}

void GeneralSettingsPage::clean()
{
    if (!isActivated())
        return;

    for (SettingsWidget* widget : widgets())
        widget->clean();
}

void GeneralSettingsPage::apply()
{
    if (!isActivated())
        return;

    for (SettingsWidget* widget : widgets())
        widget->apply();
}

QString GeneralSettingsPage::title() const
{
    return tr("General");
}

QIcon GeneralSettingsPage::icon() const
{
    return QIcon(":/images/settings/general.png");
}

bool GeneralSettingsPage::containsWord(const QString& word) const
{
    // TODO
}
