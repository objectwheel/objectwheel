#include <settingswidget.h>
#include <QBoxLayout>

SettingsWidget::SettingsWidget(QWidget* parent) : QScrollArea(parent)
  , m_marked(false)
  , m_activated(false)
{
    setWidgetResizable(true);
    setWidget(new QWidget(this));
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
    widget()->setAutoFillBackground(false);

    auto contentLayout = new QBoxLayout(QBoxLayout::TopToBottom, widget());
    contentLayout->setSpacing(6);
    contentLayout->setContentsMargins(4, 4, 4, 4);
}

void SettingsWidget::mark(bool marked)
{
    m_marked = marked;
}

bool SettingsWidget::isMarked() const
{
    return m_marked;
}

void SettingsWidget::activate(bool activated)
{
    m_activated = activated;
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
