#include <toolboxsettingswidget.h>
#include <toolboxsettings.h>
#include <designersettings.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>

ToolboxSettingsWidget::ToolboxSettingsWidget(QWidget* parent) : SettingsWidget(parent)
  , m_appearanceGroup(new QGroupBox(contentWidget()))
  , m_alternatingRowColorsLabel(new QLabel(m_appearanceGroup))
  , m_textElideModeLabel(new QLabel(m_appearanceGroup))
  , m_iconSizeLabel(new QLabel(m_appearanceGroup))
  , m_alternatingRowColorsCheckBox(new QCheckBox(m_appearanceGroup))
  , m_textElideModeBox(new QComboBox(m_appearanceGroup))
  , m_iconSizeBox(new QComboBox(m_appearanceGroup))
{
    contentLayout()->addWidget(m_appearanceGroup);
    contentLayout()->addStretch();

    /****/

    auto appearanceLayout = new QGridLayout(m_appearanceGroup);
    appearanceLayout->setSpacing(6);
    appearanceLayout->setContentsMargins(6, 6, 6, 6);
    appearanceLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    appearanceLayout->addWidget(m_alternatingRowColorsLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    appearanceLayout->addWidget(m_textElideModeLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    appearanceLayout->addWidget(m_iconSizeLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
    appearanceLayout->addWidget(m_alternatingRowColorsCheckBox, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    appearanceLayout->addWidget(m_textElideModeBox, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
    appearanceLayout->addWidget(m_iconSizeBox, 2, 2, Qt::AlignLeft | Qt::AlignVCenter);

    appearanceLayout->setColumnStretch(3, 1);
    appearanceLayout->setColumnMinimumWidth(1, 20);

    m_appearanceGroup->setTitle(tr("Appearance"));

    m_alternatingRowColorsLabel->setText(tr("Alternating row colors") + ":");
    m_textElideModeLabel->setText(tr("Text elide mode") + ":");
    m_iconSizeLabel->setText(tr("Icon size (px)") + ":");

    m_alternatingRowColorsCheckBox->setText(tr("Paint rows with alternating colors"));
    m_textElideModeBox->setToolTip(tr("Text elide mode for the toolbox items when displaying texts don't fit"));
    m_iconSizeBox->setToolTip(tr("Icon size for the toolbox items (in pixels)"));

    m_alternatingRowColorsCheckBox->setCursor(Qt::PointingHandCursor);
    m_textElideModeBox->setCursor(Qt::PointingHandCursor);
    m_iconSizeBox->setCursor(Qt::PointingHandCursor);

    /****/

    fill();

    activate();
    revert();
}

void ToolboxSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    ToolboxSettings* settings = DesignerSettings::toolboxSettings();
    /****/
    settings->enableAlternatingRowColors = m_alternatingRowColorsCheckBox->isChecked();
    settings->textElideMode = m_textElideModeBox->currentIndex();
    settings->iconSize = m_iconSizeBox->currentText().toInt();
    settings->write();
}

void ToolboxSettingsWidget::revert()
{
    if (!isActivated())
        return;

    activate(false);

    const ToolboxSettings* settings = DesignerSettings::toolboxSettings();
    /****/
    m_alternatingRowColorsCheckBox->setChecked(settings->enableAlternatingRowColors);
    m_textElideModeBox->setCurrentIndex(settings->textElideMode);
    m_iconSizeBox->setCurrentText(QString::number(settings->iconSize));
}

void ToolboxSettingsWidget::reset()
{
    DesignerSettings::toolboxSettings()->reset();
    DesignerSettings::toolboxSettings()->write();
    activate();
    revert();
}

QIcon ToolboxSettingsWidget::icon() const
{
    return QIcon(":/images/settings/toolbox.svg");
}

QString ToolboxSettingsWidget::title() const
{
    return tr("Toolbox");
}

bool ToolboxSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_appearanceGroup->title().contains(word, Qt::CaseInsensitive)
            || m_alternatingRowColorsLabel->text().contains(word, Qt::CaseInsensitive)
            || m_textElideModeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_iconSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_textElideModeBox->toolTip().contains(word, Qt::CaseInsensitive)
            || m_alternatingRowColorsCheckBox->text().contains(word, Qt::CaseInsensitive)
            || UtilityFunctions::comboContainsWord(m_textElideModeBox, word);
}

void ToolboxSettingsWidget::fill()
{
    m_textElideModeBox->addItem("Elide Left");
    m_textElideModeBox->addItem("Elide Right");
    m_textElideModeBox->addItem("Elide Middle");
    m_textElideModeBox->addItem("Elide None");

    m_iconSizeBox->addItem("8");
    m_iconSizeBox->addItem("12");
    m_iconSizeBox->addItem("16");
    m_iconSizeBox->addItem("20");
    m_iconSizeBox->addItem("24");
    m_iconSizeBox->addItem("28");
    m_iconSizeBox->addItem("32");
    m_iconSizeBox->addItem("36");
    m_iconSizeBox->addItem("40");
    m_iconSizeBox->addItem("44");
    m_iconSizeBox->addItem("48");
}
