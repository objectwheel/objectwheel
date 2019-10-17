#include <designersettingspage.h>
#include <scenesettingswidget.h>
#include <toolboxsettingswidget.h>
#include <navigatorsettingswidget.h>
#include <QIcon>

DesignerSettingsPage::DesignerSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("Designer"));
    addWidget(new SceneSettingsWidget(this));
    addWidget(new ToolboxSettingsWidget(this));
    addWidget(new NavigatorSettingsWidget(this));
}

QIcon DesignerSettingsPage::icon() const
{
    return QIcon(":/images/settings/designer.svg");
}
