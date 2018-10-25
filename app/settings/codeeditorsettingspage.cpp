#include <codeeditorsettingspage.h>
#include <fontsettingswidget.h>
#include <QIcon>

CodeEditorSettingsPage::CodeEditorSettingsPage(QWidget* parent) : SettingsPage(parent)
{
    setTitle(tr("Code Editor"));
    addWidget(new FontSettingsWidget(this));
}

QIcon CodeEditorSettingsPage::icon() const
{
    return QIcon(":/images/settings/codeeditor.png");
}

bool CodeEditorSettingsPage::containsWord(const QString& /*word*/) const
{
    // TODO
    return false;
}
