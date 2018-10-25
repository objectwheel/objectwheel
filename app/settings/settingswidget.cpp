#include <settingswidget.h>
#include <QVBoxLayout>

SettingsWidget::SettingsWidget(QWidget *parent) : QScrollArea(parent)
  , m_activated(false)
  , m_contentWidget(new QWidget(this))
  , m_contentLayout(new QVBoxLayout(m_contentWidget))
{
    m_contentLayout->setSpacing(10);
    m_contentLayout->setContentsMargins(6, 6, 6, 6);

    setWidgetResizable(true);
    setWidget(m_contentWidget);
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    m_contentWidget->setAutoFillBackground(false);
}

void SettingsWidget::activate(bool activate)
{
    m_activated = activate;
}

bool SettingsWidget::isActivated() const
{
    return m_activated;
}

QWidget* SettingsWidget::contentWidget() const
{
    return m_contentWidget;
}

QVBoxLayout* SettingsWidget::contentLayout() const
{
    return m_contentLayout;
}