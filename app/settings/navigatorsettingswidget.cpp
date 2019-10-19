#include <navigatorsettingswidget.h>
#include <navigatorsettings.h>
#include <designersettings.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>

NavigatorSettingsWidget::NavigatorSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_functionGroup(new QGroupBox(contentWidget()))
  , m_itemDoubleClickActionLabel(new QLabel(m_functionGroup))
  , m_itemDoubleClickActionBox(new QComboBox(m_functionGroup))
{
    contentLayout()->addWidget(m_functionGroup);
    contentLayout()->addStretch();

    /****/

    auto functionLayout = new QGridLayout(m_functionGroup);
    functionLayout->setSpacing(8);
    functionLayout->setContentsMargins(6, 6, 6, 6);
    functionLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    functionLayout->addWidget(m_itemDoubleClickActionLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    functionLayout->addWidget(m_itemDoubleClickActionBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);

    functionLayout->setColumnStretch(3, 1);
    functionLayout->setColumnMinimumWidth(1, 20);

    m_functionGroup->setTitle(tr("Function"));

    m_itemDoubleClickActionLabel->setText(tr("Item double click action") + ":");

    m_itemDoubleClickActionBox->setToolTip(tr("Change default action for the double clicks on items"));

    m_itemDoubleClickActionBox->setCursor(Qt::PointingHandCursor);
    /****/

    fill();

    activate();
    revert();
}

void NavigatorSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    NavigatorSettings* settings = DesignerSettings::navigatorSettings();
    /****/
    settings->itemDoubleClickAction = m_itemDoubleClickActionBox->currentIndex();
    settings->write();
}

void NavigatorSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);

    const NavigatorSettings* settings = DesignerSettings::navigatorSettings();
    /****/
    m_itemDoubleClickActionBox->setCurrentIndex(settings->itemDoubleClickAction);
}

void NavigatorSettingsWidget::reset()
{
    DesignerSettings::navigatorSettings()->reset();
    DesignerSettings::navigatorSettings()->write();
    activate();
    revert();
}

QIcon NavigatorSettingsWidget::icon() const
{
    return QIcon(":/images/settings/navigator.svg");
}

QString NavigatorSettingsWidget::title() const
{
    return tr("Navigator");
}

bool NavigatorSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_functionGroup->title().contains(word, Qt::CaseInsensitive)
            || m_itemDoubleClickActionLabel->text().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_itemDoubleClickActionBox, word);
}

void NavigatorSettingsWidget::fill()
{
    m_itemDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/edit-anchors.svg")),
                                        tr("Edit Anchors Action"));
    m_itemDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/view-source-code.svg")),
                                        tr("View Source Code Action"));
    m_itemDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/go-to-slot.svg")),
                                        tr("Go to Slot Action"));
}
