#include <settingspage.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget* parent) : QWidget(parent)
  , m_tabWidget(new QTabWidget(this))
  , m_titleLabel(new QLabel(this))
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_tabWidget);

    UtilityFunctions::adjustFontPixelSize(m_titleLabel, 1);
    UtilityFunctions::adjustFontWeight(m_titleLabel, QFont::DemiBold);

    QMetaObject::invokeMethod(this, [=] { m_titleLabel->setText(title()); });
}

void SettingsPage::addWidget(const QString& title, QWidget* widget)
{
    m_tabWidget->addTab(widget, title);
}

QSize SettingsPage::sizeHint() const
{
    return {200, 200};
}
