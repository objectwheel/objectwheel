#include <designersettingspage.h>
#include <scenesettingswidget.h>
#include <toolboxsettingswidget.h>
#include <QIcon>

DesignerSettingsPage::DesignerSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("Designer"));
    addWidget(new SceneSettingsWidget(this));
    addWidget(new ToolboxSettingsWidget(this));
}

QIcon DesignerSettingsPage::icon() const
{
    return QIcon(":/images/settings/designer.svg");
}
