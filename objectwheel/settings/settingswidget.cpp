#include <settingswidget.h>
#include <QBoxLayout>

SettingsWidget::SettingsWidget(QWidget* parent) : QScrollArea(parent)
  , m_activated(false)
{
    setWidgetResizable(true);
    setWidget(new QWidget(this));
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    widget()->setAutoFillBackground(false);

    auto contentLayout = new QBoxLayout(QBoxLayout::TopToBottom, widget());
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(6, 6, 6, 6);
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
    return widget();
}

QBoxLayout* SettingsWidget::contentLayout() const
{
    return static_cast<QBoxLayout*>(widget()->layout());
}
