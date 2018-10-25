#include <settingspage.h>
#include <utilityfunctions.h>
#include <settingswidget.h>

#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent)
  , m_tabWidget(new QTabWidget(this))
  , m_titleLabel(new QLabel(this))
{
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_tabWidget);

    UtilityFunctions::adjustFontPixelSize(m_titleLabel, 1);
    UtilityFunctions::adjustFontWeight(m_titleLabel, QFont::DemiBold);

    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, [=] (int index) {
        if (index < 0)
            return;
        if (SettingsWidget* widget = qobject_cast<SettingsWidget*>(m_tabWidget->widget(index)))
            widget->activate();
    });
}

void SettingsPage::reset()
{
    for (SettingsWidget* widget : widgets())
        widget->reset();
    activateCurrent();
}

void SettingsPage::apply()
{
    for (SettingsWidget* widget : widgets())
        widget->apply();
    activateCurrent();
}

void SettingsPage::activateCurrent()
{
    if (SettingsWidget* widget = qobject_cast<SettingsWidget*>(m_tabWidget->currentWidget()))
        widget->activate();
}

void SettingsPage::setTitle(const QString& title)
{
    m_titleLabel->setText(title);
}

QString SettingsPage::title() const
{
    return m_titleLabel->text();
}

void SettingsPage::addWidget(SettingsWidget* widget)
{
    m_tabWidget->addTab(widget, widget->icon(), widget->title());
}

QList<SettingsWidget*> SettingsPage::widgets() const
{
    QList<SettingsWidget*> widgets;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (SettingsWidget* w = qobject_cast<SettingsWidget*>(m_tabWidget->widget(i)))
            widgets.append(w);
    }
    return widgets;
}
