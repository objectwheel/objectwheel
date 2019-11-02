#include <designersettingspage.h>
#include <scenesettingswidget.h>
#include <toolboxsettingswidget.h>
#include <controlssettingswidget.h>
#include <QIcon>

DesignerSettingsPage::DesignerSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("Designer"));
    addWidget(new SceneSettingsWidget(this));
    addWidget(new ToolboxSettingsWidget(this));
    addWidget(new ControlsSettingsWidget(this));
}

QIcon DesignerSettingsPage::icon() const
{
    return QIcon(":/images/settings/designer.svg");
}
