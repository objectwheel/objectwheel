#include <fontcolorssettingswidget.h>
#include <codeeditorsettings.h>

#include <texteditor/colorschemeedit.h>

#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QApplication>

namespace {

const char* g_colorSchemeFiles[] = {":/styles/creator-dark.xml",
                                    ":/styles/dark.xml",
                                    ":/styles/default.xml",
                                    ":/styles/default_classic.xml",
                                    ":/styles/grayscale.xml",
                                    ":/styles/inkpot.xml",
                                    ":/styles/intellij.xml",
                                    ":/styles/modnokai_night_shift_v2.xml",
                                    ":/styles/solarized-dark.xml",
                                    ":/styles/solarized-light.xml"};
const char* g_colorSchemeNames[] = {"Qt Dark",
                                    "Dark",
                                    "Default",
                                    "Default Classic",
                                    "Grayscale",
                                    "Inkpot",
                                    "IntelliJ IDEA",
                                    "Modnokai Night Shift v2",
                                    "Solarized Dark",
                                    "Solarized Light"};

void addColorSchemes(QComboBox* comboBox)
{
    for (size_t i = 0; i < sizeof(g_colorSchemeNames) / sizeof(g_colorSchemeNames[0]); ++i)
        comboBox->addItem(QObject::QObject::tr(g_colorSchemeNames[i]), g_colorSchemeFiles[i]);
}
}

struct ColorSchemeEntry
{
    ColorSchemeEntry(const QString &fileName,
                     bool readOnly):
        fileName(fileName),
        name(TextEditor::ColorScheme::readNameOfScheme(fileName)),
        readOnly(readOnly)
    { }

    QString fileName;
    QString name;
    QString id;
    bool readOnly;
};

class SchemeListModel : public QAbstractListModel
{
public:
    SchemeListModel(QObject *parent = 0):
        QAbstractListModel(parent)
    {
    }

    int rowCount(const QModelIndex &parent) const
    { return parent.isValid() ? 0 : m_colorSchemes.size(); }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (role == Qt::DisplayRole)
            return m_colorSchemes.at(index.row()).name;

        return QVariant();
    }

    void removeColorScheme(int index)
    {
        beginRemoveRows(QModelIndex(), index, index);
        m_colorSchemes.removeAt(index);
        endRemoveRows();
    }

    void setColorSchemes(const QList<ColorSchemeEntry> &colorSchemes)
    {
        beginResetModel();
        m_colorSchemes = colorSchemes;
        endResetModel();
    }

    const ColorSchemeEntry &colorSchemeAt(int index) const
    { return m_colorSchemes.at(index); }

private:
    QList<ColorSchemeEntry> m_colorSchemes;
};

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
  /****/
  , m_schemeListModel(new SchemeListModel)
  , m_colorSchemeGroup(new QGroupBox(contentWidget()))
  , m_colorSchemeLayout(new QGridLayout(m_colorSchemeGroup))
  , m_colorSchemeBox(new QComboBox(m_colorSchemeGroup))
  , m_colorSchemeCopyButton(new QPushButton(m_colorSchemeGroup))
  , m_colorSchemeDeleteButton(new QPushButton(m_colorSchemeGroup))
  , m_colorSchemeEdit(new TextEditor::Internal::ColorSchemeEdit(m_colorSchemeGroup))
{
    contentLayout()->addWidget(m_fontGroup);
    contentLayout()->addWidget(m_colorSchemeGroup);
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

    m_fontGroup->setTitle(QObject::tr("Font"));
    m_fontFamilyLabel->setText(QObject::tr("Family") + ":");
    m_fontFamilyBox->addItems(QFontDatabase().families());
    m_fontSizeLabel->setText(QObject::tr("Size") + ":");
    m_fontSizeBox->addItems({"8", "9", "10", "11", "12", "13", "14", "15", "16",
                             "18", "24", "36", "48", "64", "72", "96", "144"});
    m_fontAntialiasingBox->setText(QObject::tr("Prefer antialiasing"));
    m_fontThickBox->setText(QObject::tr("Prefer thicker"));
    m_fontResetButton->setText(QObject::tr("Reset"));

    m_fontFamilyBox->setToolTip(QObject::tr("Chage font family"));
    m_fontSizeBox->setToolTip(QObject::tr("Chage font pixel size"));
    m_fontAntialiasingBox->setToolTip(QObject::tr("Enable font antialiasing"));
    m_fontThickBox->setToolTip(QObject::tr("Enable text thickness increasing"));
    m_fontResetButton->setToolTip(QObject::tr("Reset font settings to default"));

    m_fontFamilyBox->setCursor(Qt::PointingHandCursor);
    m_fontSizeBox->setCursor(Qt::PointingHandCursor);
    m_fontAntialiasingBox->setCursor(Qt::PointingHandCursor);
    m_fontThickBox->setCursor(Qt::PointingHandCursor);
    m_fontResetButton->setCursor(Qt::PointingHandCursor);

    /****/

    m_colorSchemeLayout->setSpacing(8);
    m_colorSchemeLayout->setContentsMargins(6, 6, 6, 6);
    m_colorSchemeLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_colorSchemeLayout->addWidget(m_colorSchemeBox, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_colorSchemeLayout->addWidget(m_colorSchemeCopyButton, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_colorSchemeLayout->addWidget(m_colorSchemeDeleteButton, 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
    m_colorSchemeLayout->addWidget(m_colorSchemeEdit, 1, 0, 1, 3, Qt::AlignLeft | Qt::AlignVCenter);

    m_colorSchemeGroup->setTitle(QObject::tr("Color Scheme"));
    m_colorSchemeDeleteButton->setText(QObject::tr("Delete"));
    m_colorSchemeCopyButton->setText(QObject::tr("Copy") + "...");

    m_colorSchemeBox->setToolTip(QObject::tr("Chage code color scheme"));
    m_colorSchemeCopyButton->setToolTip(QObject::tr("Copy and create new color scheme from current scheme"));
    m_colorSchemeDeleteButton->setToolTip(QObject::tr("Delete current color scheme"));

    m_colorSchemeBox->setCursor(Qt::PointingHandCursor);
    m_colorSchemeCopyButton->setCursor(Qt::PointingHandCursor);
    m_colorSchemeDeleteButton->setCursor(Qt::PointingHandCursor);

    /****/

    addColorSchemes(m_colorSchemeBox);

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
    return QObject::tr("Font") + " && " + QObject::tr("Colors");
}

bool FontColorsSettingsWidget::containsWord(const QString& word) const
{
    return title().contains(word, Qt::CaseInsensitive)
            || m_fontFamilyLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontSizeLabel->text().contains(word, Qt::CaseInsensitive)
            || m_fontAntialiasingBox->text().contains(word, Qt::CaseInsensitive)
            || m_fontThickBox->text().contains(word, Qt::CaseInsensitive);
}