#include <codeeditorsettingspage.h>
#include <fontcolorssettingswidget.h>
#include <QIcon>

CodeEditorSettingsPage::CodeEditorSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("Code Editor"));
    addWidget(new FontColorsSettingsWidget(this));
}

QIcon CodeEditorSettingsPage::icon() const
{
    return QIcon(":/images/settings/codeeditor.png");
}
