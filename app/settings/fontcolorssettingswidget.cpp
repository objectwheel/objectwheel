#include <fontcolorssettingswidget.h>
#include <fontcolorssettings.h>
#include <codeeditorsettings.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QFontDatabase>

FontColorsSettingsWidget::FontColorsSettingsWidget(QWidget *parent) : SettingsWidget(parent)
  , m_fontGroup(new QGroupBox(contentWidget()))
  , m_fontLayout(new QVBoxLayout(m_fontGroup))
  , m_fontFamilyLabel(new QLabel(m_fontGroup))
  , m_fontSizeLabel(new QLabel(m_fontGroup))
  , m_fontFamilyBox(new QComboBox(m_fontGroup))
  , m_fontSizeBox(new QComboBox(m_fontGroup))
  , m_fontAntialiasingBox(new QCheckBox(m_fontGroup))
  , m_fontThickBox(new QCheckBox(m_fontGroup))
  , m_fontResetButton(new QPushButton(m_fontGroup))
{
    contentLayout()->addWidget(m_fontGroup);
    contentLayout()->addStretch();

    /****/

    auto hb1 = new QHBoxLayout;
    hb1->setSpacing(8);
    hb1->setContentsMargins(0, 0, 0, 0);
    hb1->addWidget(m_fontFamilyLabel);
    hb1->addWidget(m_fontFamilyBox);
    hb1->addSpacing(20);
    hb1->addWidget(m_fontSizeLabel);
    hb1->addWidget(m_fontSizeBox);
    hb1->addSpacing(30);
    hb1->addWidget(m_fontResetButton);
    hb1->addStretch();
    auto hb2 = new QHBoxLayout;
    hb2->setSpacing(8);
    hb2->setContentsMargins(0, 0, 0, 0);
    hb2->addWidget(m_fontAntialiasingBox);
    hb2->addWidget(m_fontThickBox);
    hb2->addStretch();

    m_fontLayout->setSpacing(8);
    m_fontLayout->setContentsMargins(6, 6, 6, 6);
    m_fontLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_fontLayout->addLayout(hb1);
    m_fontLayout->addLayout(hb2);

    m_fontGroup->setTitle(tr("Font"));
    m_fontFamilyLabel->setText(tr("Family") + ":");
    m_fontFamilyBox->addItems(QFontDatabase().families());
    m_fontSizeLabel->setText(tr("Size") + ":");
    m_fontSizeBox->addItems({"8", "9", "10", "11", "12", "13", "14", "15", "16",
                             "18", "24", "36", "48", "64", "72", "96", "144"});
    m_fontAntialiasingBox->setText(tr("Prefer antialiasing"));
    m_fontThickBox->setText(tr("Prefer thicker"));
    m_fontResetButton->setText(tr("Reset"));

    m_fontFamilyBox->setToolTip(tr("Chage font family"));
    m_fontSizeBox->setToolTip(tr("Chage font pixel size"));
    m_fontAntialiasingBox->setToolTip(tr("Enable font antialiasing"));
    m_fontThickBox->setToolTip(tr("Enable text thickness increasing"));
    m_fontResetButton->setToolTip(tr("Reset font settings to default"));

    m_fontFamilyBox->setCursor(Qt::PointingHandCursor);
    m_fontSizeBox->setCursor(Qt::PointingHandCursor);
    m_fontAntialiasingBox->setCursor(Qt::PointingHandCursor);
    m_fontThickBox->setCursor(Qt::PointingHandCursor);
    m_fontResetButton->setCursor(Qt::PointingHandCursor);

    /****/

    connect(m_fontResetButton, &QPushButton::clicked, this, [=] {
        const FontColorsSettings settings;
        m_fontFamilyBox->setCurrentText(settings.fontFamily);
        m_fontSizeBox->setCurrentText(QString::number(settings.fontPixelSize));
        m_fontThickBox->setChecked(settings.fontPreferThick);
        m_fontAntialiasingBox->setChecked(settings.fontPreferAntialiasing);
    });

    activate();
    reset();
}

void FontColorsSettingsWidget::apply()
{
    if (!isActivated())
        return;

    activate(false);

    FontColorsSettings* settings = CodeEditorSettings::fontColorsSettings();
    /****/
    settings->fontFamily = m_fontFamilyBox->currentText();
    settings->fontPixelSize = m_fontSizeBox->currentText().toInt();
    settings->fontPreferThick = m_fontThickBox->isChecked();
    settings->fontPreferAntialiasing = m_fontAntialiasingBox->isChecked();
    /****/
    settings->write();
}

void FontColorsSettingsWidget::reset()
{
    if (!isActivated())
        return;

    activate(false);

    const FontColorsSettings* settings = CodeEditorSettings::fontColorsSettings();
    /****/
    m_fontFamilyBox->setCurrentText(settings->fontFamily);
    m_fontSizeBox->setCurrentText(QString::number(settings->fontPixelSize));
    m_fontThickBox->setChecked(settings->fontPreferThick);
    m_fontAntialiasingBox->setChecked(settings->fontPreferAntialiasing);
}

QIcon FontColorsSettingsWidget::icon() const
{
    return QIcon(":/images/settings/fontcolors.png");
}

QString FontColorsSettingsWidget::title() const
{
    return tr("Font") + " && " + tr("Colors");
}

bool FontColorsSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_fontFamilyLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontAntialiasingBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontThickBox->text().contains(word, Qt::CaseInsensitive);
}