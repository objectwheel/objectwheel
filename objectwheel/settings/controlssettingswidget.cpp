#include <controlssettingswidget.h>
#include <controlssettings.h>
#include <designersettings.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>

ControlsSettingsWidget::ControlsSettingsWidget(QWidget* parent) : SettingsWidget(parent)
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

void ControlsSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    ControlsSettings* settings = DesignerSettings::controlsSettings();
    /****/
    settings->itemDoubleClickAction = m_itemDoubleClickActionBox->currentIndex();
    settings->write();
}

void ControlsSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);

    const ControlsSettings* settings = DesignerSettings::controlsSettings();
    /****/
    m_itemDoubleClickActionBox->setCurrentIndex(settings->itemDoubleClickAction);
}

void ControlsSettingsWidget::reset()
{
    DesignerSettings::controlsSettings()->reset();
    DesignerSettings::controlsSettings()->write();
    activate();
    revert();
}

QIcon ControlsSettingsWidget::icon() const
{
    return QIcon(":/images/settings/controls.svg");
}

QString ControlsSettingsWidget::title() const
{
    return tr("Controls");
}

bool ControlsSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_functionGroup->title().contains(word, Qt::CaseInsensitive)
            || m_itemDoubleClickActionLabel->text().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_itemDoubleClickActionBox, word);
}

void ControlsSettingsWidget::fill()
{
    m_itemDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/edit-anchors.svg")),
                                        tr("Edit Anchors Action"));
    m_itemDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/view-source-code.svg")),
                                        tr("View Source Code Action"));
    m_itemDoubleClickActionBox->addItem(QIcon(QStringLiteral(":/images/designer/go-to-slot.svg")),
                                        tr("Go to Slot Action"));
}
