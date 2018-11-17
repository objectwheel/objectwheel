#include <settingspage.h>
#include <utilityfunctions.h>
#include <settingswidget.h>

#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWindow>
#include <QTabBar>

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent)
  , m_tabWidget(new QTabWidget(this))
  , m_titleLabel(new QLabel(this))
{
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_tabWidget);

    m_tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);

    UtilityFunctions::adjustFontPixelSize(m_titleLabel, 1);
    UtilityFunctions::adjustFontWeight(m_titleLabel, QFont::DemiBold);

    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, [=] (int index) {
        if (index < 0)
            return;
        if (!UtilityFunctions::window(this) || !UtilityFunctions::window(this)->isVisible())
            return;
        if (SettingsWidget* widget = qobject_cast<SettingsWidget*>(m_tabWidget->widget(index)))
            widget->activate();
    });
}

void SettingsPage::reset()
{
    for (SettingsWidget* widget : widgets())
        widget->reset();
}
void SettingsPage::apply()
{
    for (SettingsWidget* widget : widgets())
        widget->apply();
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

bool SettingsPage::containsWord(const QString& word) const
{
    if (title().contains(word, Qt::CaseInsensitive))
        return true;
    for (SettingsWidget* widget : widgets()) {
        if (widget->containsWord(word))
            return true;
    }
    return false;
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
