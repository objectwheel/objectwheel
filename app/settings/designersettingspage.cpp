#include <designersettingspage.h>
#include <scenesettingswidget.h>
#include <QIcon>

DesignerSettingsPage::DesignerSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("Designer"));
    addWidget(new SceneSettingsWidget(this));
}

QIcon DesignerSettingsPage::icon() const
{
    return QIcon(":/images/settings/designer.svg");
}
