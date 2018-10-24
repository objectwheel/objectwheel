#include <settingswidget.h>
#include <QEvent>
#include <QVBoxLayout>

SettingsWidget::SettingsWidget(QWidget *parent) : QScrollArea(parent)
  , m_activated(false)
  , m_contentWidget(new QWidget(this))
  , m_contentLayout(new QVBoxLayout(m_contentWidget))
{
    m_contentLayout->setSpacing(8);
    m_contentLayout->setContentsMargins(6, 6, 6, 6);

    setWidgetResizable(true);
    setWidget(m_contentWidget);
    setFrameShape(QFrame::NoFrame);

    QPalette c(m_contentWidget->palette());
    QPalette p(m_contentWidget->palette());
    p.setColor(QPalette::Window, Qt::transparent);

//    m_contentWidget->setAutoFillBackground(false);
//    setPalette(p);
//    m_contentWidget->setPalette(c);
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

bool SettingsWidget::event(QEvent* e)
{
    if (e->type() == QEvent::Paint) {
        e->accept();
        return true;
    }
    return QScrollArea::event(e);
}

void SettingsWidget::paintEvent(QPaintEvent*)
{
}

#include "settingswidget.moc"
