#include <settingswidget.h>

SettingsWidget::SettingsWidget(QWidget *parent) : QAbstractScrollArea(parent)
  , m_activated(false)
{
}

void SettingsWidget::activate(bool activate)
{
    m_activated = activate;
}

bool SettingsWidget::isActivated() const
{
    return m_activated;
}