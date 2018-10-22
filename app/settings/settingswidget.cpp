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

QString SettingsWidget::title() const
{
    return m_title;
}

void SettingsWidget::setTitle(const QString& title)
{
    m_title = title;
}