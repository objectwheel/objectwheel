#include <generalsettingspage.h>
#include <QIcon>

GeneralSettingsPage::GeneralSettingsPage(QWidget* parent) : SettingsPage(parent)
{
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
