#include <themechooserwidget.h>
#include <dpr.h>
#include <fit.h>
#include <delayer.h>
#include <QtQuickControls2>
#include <QtWidgets>

namespace {
    extern const QStringList STYLES;
    extern const QStringList STYLES_V2;
    extern const QStringList MATERIAL_THEMES;
    extern const QStringList UNIVERSAL_THEMES;
    extern const QStringList MATERIAL_COLORS_LIGHT;
    extern const QStringList MATERIAL_COLORS_DARK;
    extern const QStringList UNIVERSAL_COLORS;
}

ThemeChooserWidget::ThemeChooserWidget(const Version& version, QWidget *parent) : QWidget(parent)
  , m_version(version)
  , m_stylesLabel(new QLabel)
  , m_stylesCombo(new QComboBox)
  , m_seeRunningButton(new QPushButton)
  , m_themesLabel(new QLabel)
  , m_themesCombo(new QComboBox)
  , m_detailsLabel(new QLabel)
  , m_accentDetailLabel(new QLabel)
  , m_primaryDetailLabel(new QLabel)
  , m_foregroundDetailLabel(new QLabel)
  , m_backgroundDetailLabel(new QLabel)
  , m_accentColorsCombo(new QComboBox)
  , m_primaryColorsCombo(new QComboBox)
  , m_foregroundColorsCombo(new QComboBox)
  , m_backgroundColorsCombo(new QComboBox)
  , m_accentColorLabel(new QLabel)
  , m_primaryColorLabel(new QLabel)
  , m_foregroundColorLabel(new QLabel)
  , m_backgroundColorLabel(new QLabel)
  , m_accentColorButton(new QToolButton)
  , m_primaryColorButton(new QToolButton)
  , m_foregroundColorButton(new QToolButton)
  , m_backgroundColorButton(new QToolButton)
  , m_customizationLabel(new QLabel)
  , m_customizationPicture(new QLabel)
  , m_previewLabel(new QLabel)
  , m_previewPicture(new QLabel)
  , m_gridLayout(new QGridLayout(this))
{
    QFont sectionsFont;
    sectionsFont.setWeight(QFont::DemiBold);

    m_stylesLabel->setText(tr("Style:"));
    m_stylesLabel->setStyleSheet("color: #2E3A41");
    m_stylesLabel->setFont(sectionsFont);

    m_stylesCombo->addItems(m_version == V1 ? STYLES : STYLES_V2);
    m_stylesCombo->setFixedWidth(fit::fx(150));

    m_seeRunningButton->setText(tr("See Running"));
    m_seeRunningButton->setDefault(true);

    if (m_version == V2) {
        m_themesLabel->setText(tr("Theme:"));
        m_themesLabel->setStyleSheet("color: #2E3A41");
        m_themesLabel->setFont(sectionsFont);

        m_themesCombo->setCurrentText("None");
        m_themesCombo->setFixedWidth(fit::fx(150));

        m_detailsLabel->setText(tr("Details:"));
        m_detailsLabel->setStyleSheet("color: #2E3A41");
        m_detailsLabel->setFont(sectionsFont);

        m_accentDetailLabel->setText(tr("Accent color:"));
        m_accentDetailLabel->setStyleSheet("color: #2E3A41");

        m_primaryDetailLabel->setText(tr("Primary color:"));
        m_primaryDetailLabel->setStyleSheet("color: #2E3A41");

        m_foregroundDetailLabel->setText(tr("Foreground color:"));
        m_foregroundDetailLabel->setStyleSheet("color: #2E3A41");

        m_backgroundDetailLabel->setText(tr("Background color:"));
        m_backgroundDetailLabel->setStyleSheet("color: #2E3A41");

        m_accentColorsCombo->setFixedWidth(fit::fx(200));

        m_primaryColorsCombo->setFixedWidth(fit::fx(200));

        m_foregroundColorsCombo->setFixedWidth(fit::fx(200));

        m_backgroundColorsCombo->setFixedWidth(fit::fx(200));

        m_accentColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_accentColorLabel->setFrameShape(QLabel::StyledPanel);
        m_accentColorLabel->setFrameShadow(QLabel::Plain);

        m_primaryColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_primaryColorLabel->setFrameShape(QLabel::StyledPanel);
        m_primaryColorLabel->setFrameShadow(QLabel::Plain);

        m_foregroundColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_foregroundColorLabel->setFrameShape(QLabel::StyledPanel);
        m_foregroundColorLabel->setFrameShadow(QLabel::Plain);

        m_backgroundColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_backgroundColorLabel->setFrameShape(QLabel::StyledPanel);
        m_backgroundColorLabel->setFrameShadow(QLabel::Plain);

        m_accentColorButton->setText("...");

        m_primaryColorButton->setText("...");

        m_foregroundColorButton->setText("...");

        m_backgroundColorButton->setText("...");

        m_customizationLabel->setText(tr("Customization:"));
        m_customizationLabel->setStyleSheet("color: #2E3A41");
        m_customizationLabel->setFont(sectionsFont);

        m_customizationPicture->setScaledContents(false);
        m_customizationPicture->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    }

    m_previewLabel->setText(tr("Preview:"));
    m_previewLabel->setStyleSheet("color: #2E3A41");
    m_previewLabel->setFont(sectionsFont);

    m_previewPicture->setScaledContents(true);
    m_previewPicture->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_previewPicture->setFixedSize(360, 600);
    m_previewPicture->setFrameShape(QLabel::StyledPanel);
    m_previewPicture->setFrameShadow(QLabel::Plain);

    m_gridLayout->addWidget(m_stylesLabel, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridLayout->addWidget(m_stylesCombo, 1, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    m_gridLayout->addWidget(m_seeRunningButton, 2, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);

    if (m_version == V2) {
        m_gridLayout->addWidget(m_themesLabel, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_themesCombo, 1, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

        m_gridLayout->addWidget(m_detailsLabel, 0, 2, 1, 3, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_accentDetailLabel, 1, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_foregroundDetailLabel, 2, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_backgroundDetailLabel, 3, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_primaryDetailLabel, 4, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

        m_gridLayout->addWidget(m_customizationLabel, 6, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_customizationPicture, 8, 2, 1, 3, Qt::AlignLeft | Qt::AlignTop);

        m_gridLayout->addWidget(m_accentColorsCombo, 1, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_foregroundColorsCombo, 2, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_backgroundColorsCombo, 3, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_primaryColorsCombo, 4, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

        m_gridLayout->addWidget(m_accentColorLabel, 1, 4, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_foregroundColorLabel, 2, 4, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_backgroundColorLabel, 3, 4, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_primaryColorLabel, 4, 4, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

        m_gridLayout->addWidget(m_accentColorButton, 1, 5, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_foregroundColorButton, 2, 5, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_backgroundColorButton, 3, 5, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        m_gridLayout->addWidget(m_primaryColorButton, 4, 5, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    }

    m_gridLayout->addWidget(m_previewLabel, 0, 6, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    m_gridLayout->addWidget(m_previewPicture, 1, 6, 8, 1, Qt::AlignLeft | Qt::AlignTop);

    m_gridLayout->setRowStretch(8, 1);
    m_gridLayout->setColumnStretch(7, 1);


    if (m_version == V2) {
        m_themesCombo->addItem(tr("Unavailable"));
        m_themesCombo->setDisabled(true);

        m_accentColorsCombo->addItem(tr("Unavailable"));
        m_primaryColorsCombo->addItem(tr("Unavailable"));
        m_foregroundColorsCombo->addItem(tr("Unavailable"));
        m_backgroundColorsCombo->addItem(tr("Unavailable"));

        m_accentColorsCombo->setDisabled(true);
        m_primaryColorsCombo->setDisabled(true);
        m_foregroundColorsCombo->setDisabled(true);
        m_backgroundColorsCombo->setDisabled(true);

        m_accentColorButton->setDisabled(true);
        m_primaryColorButton->setDisabled(true);
        m_foregroundColorButton->setDisabled(true);
        m_backgroundColorButton->setDisabled(true);
    }

    m_customizationLabel->setHidden(true);
    m_customizationPicture->setFixedSize(420 / DPR, 420 / DPR);

    if (m_version == V2) {
        connect(m_accentColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_primaryColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_foregroundColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_backgroundColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_themesCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
    }

    connect(m_stylesCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);

    if (m_version == V2) {
        connect(m_accentColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_accentColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty()) {
                m_accentColorLabel->setStyleSheet("background: transparent");
                return;
            }

            m_accentColorLabel->setStyleSheet(QString("background: %1").arg(colorName));
        });

        connect(m_primaryColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_primaryColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty()) {
                m_primaryColorLabel->setStyleSheet("background: transparent");
                return;
            }

            m_primaryColorLabel->setStyleSheet(QString("background: %1").arg(colorName));
        });

        connect(m_foregroundColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_foregroundColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty()) {
                m_foregroundColorLabel->setStyleSheet("background: transparent");
                return;
            }

            m_foregroundColorLabel->setStyleSheet(QString("background: %1").arg(colorName));
        });

        connect(m_backgroundColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_backgroundColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty()) {
                m_backgroundColorLabel->setStyleSheet("background: transparent");
                return;
            }

            m_backgroundColorLabel->setStyleSheet(QString("background: %1").arg(colorName));
        });
    }

    if (m_version ==  V2) {
        connect(m_themesCombo, &QComboBox::currentTextChanged, [=] {
            m_accentColorsCombo->clear();
            m_primaryColorsCombo->clear();
            m_foregroundColorsCombo->clear();
            m_backgroundColorsCombo->clear();

            if (m_themesCombo->currentText() == tr("Unavailable")) {
                m_accentColorsCombo->addItem(tr("Unavailable"));
                m_primaryColorsCombo->addItem(tr("Unavailable"));
                m_foregroundColorsCombo->addItem(tr("Unavailable"));
                m_backgroundColorsCombo->addItem(tr("Unavailable"));

                m_accentColorsCombo->setDisabled(true);
                m_primaryColorsCombo->setDisabled(true);
                m_foregroundColorsCombo->setDisabled(true);
                m_backgroundColorsCombo->setDisabled(true);

                m_accentColorButton->setDisabled(true);
                m_primaryColorButton->setDisabled(true);
                m_foregroundColorButton->setDisabled(true);
                m_backgroundColorButton->setDisabled(true);
                return;
            } else {
                m_accentColorsCombo->setEnabled(true);
                m_primaryColorsCombo->setEnabled(true);
                m_foregroundColorsCombo->setEnabled(true);
                m_backgroundColorsCombo->setEnabled(true);

                m_accentColorButton->setEnabled(true);
                m_primaryColorButton->setEnabled(true);
                m_foregroundColorButton->setEnabled(true);
                m_backgroundColorButton->setEnabled(true);
            }

            if (m_stylesCombo->currentText() == "Material") {
                for (int i = 0; i < MATERIAL_COLORS_LIGHT.size(); i++) {
                    const auto& text = m_themesCombo->currentText() == "Light" ? MATERIAL_COLORS_LIGHT[i] : MATERIAL_COLORS_DARK[i];
                    QRegularExpression exp("#.*(?=\\))");
                    QColor color(exp.match(text).captured(0));
                    m_accentColorsCombo->addItem(text);
                    m_primaryColorsCombo->addItem(text);
                    m_foregroundColorsCombo->addItem(text);
                    m_backgroundColorsCombo->addItem(text);

                    m_accentColorsCombo->setItemData(m_accentColorsCombo->count() - 1, color, Qt::DecorationRole);
                    m_primaryColorsCombo->setItemData(m_primaryColorsCombo->count() - 1, color, Qt::DecorationRole);
                    m_foregroundColorsCombo->setItemData(m_foregroundColorsCombo->count() - 1, color, Qt::DecorationRole);
                    m_backgroundColorsCombo->setItemData(m_backgroundColorsCombo->count() - 1, color, Qt::DecorationRole);
                }

                m_accentColorsCombo->setCurrentIndex(1);
                m_primaryColorsCombo->setCurrentIndex(4);
                m_foregroundColorsCombo->setCurrentIndex(19);
                m_backgroundColorsCombo->setCurrentIndex(20);
            } else if (m_stylesCombo->currentText() == "Universal") {
                m_primaryColorsCombo->addItem(tr("Unavailable"));
                m_primaryColorsCombo->setDisabled(true);
                m_primaryColorButton->setDisabled(true);

                for (int i = 0; i < UNIVERSAL_COLORS.size(); i++) {
                    const auto& text = UNIVERSAL_COLORS[i];
                    QRegularExpression exp("#.*(?=\\))");
                    QColor color(exp.match(text).captured(0));
                    m_accentColorsCombo->addItem(text);
                    m_foregroundColorsCombo->addItem(text);
                    m_backgroundColorsCombo->addItem(text);

                    m_accentColorsCombo->setItemData(m_accentColorsCombo->count() - 1, color, Qt::DecorationRole);
                    m_foregroundColorsCombo->setItemData(m_foregroundColorsCombo->count() - 1, color, Qt::DecorationRole);
                    m_backgroundColorsCombo->setItemData(m_backgroundColorsCombo->count() - 1, color, Qt::DecorationRole);
                }

                m_accentColorsCombo->setCurrentIndex(5);

                if (m_themesCombo->currentText() == "Light") {
                    m_foregroundColorsCombo->setCurrentIndex(20);
                    m_backgroundColorsCombo->setCurrentIndex(21);
                } else {
                    m_backgroundColorsCombo->setCurrentIndex(20);
                    m_foregroundColorsCombo->setCurrentIndex(21);
                }
            }
        });
}
    connect(m_stylesCombo, &QComboBox::currentTextChanged, [=] {
        m_themesCombo->clear();

        if (m_stylesCombo->currentText() != "Material" && m_stylesCombo->currentText() != "Universal" ) {
            m_themesCombo->addItem(tr("Unavailable"));
            m_themesCombo->setDisabled(true);
            m_customizationLabel->setHidden(true);
            m_customizationPicture->clear();
            return;
        } else {
            m_themesCombo->setEnabled(true);
            m_customizationLabel->setVisible(true);
        }

        if (m_stylesCombo->currentText() == "Material") {
            m_themesCombo->addItems(MATERIAL_THEMES);
            QPixmap p(":/resources/images/material.png");
            p.setDevicePixelRatio(DPR);
            m_customizationPicture->setPixmap(p);
        } else {
            QPixmap p(":/resources/images/universal.png");
            p.setDevicePixelRatio(DPR);
            m_customizationPicture->setPixmap(p);
            m_themesCombo->addItems(UNIVERSAL_THEMES);
        }
    });

    resize(1299, 1299);
}

void ThemeChooserWidget::reset()
{

}

void ThemeChooserWidget::refresh()
{
    QTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.close();

    QString json;
    if (m_version == V2) {
        QJsonObject object;
        QRegularExpression exp("#.*(?=\\))");

        auto text = m_stylesCombo->currentText();
        object.insert("style", text);

        text = m_themesCombo->currentText();
        object.insert("theme", text);

        text = m_accentColorsCombo->currentText();
        object.insert("accent", exp.match(text).captured(0));

        text = m_primaryColorsCombo->currentText();
        object.insert("primary", exp.match(text).captured(0));

        text = m_backgroundColorsCombo->currentText();
        object.insert("background", exp.match(text).captured(0));

        text = m_foregroundColorsCombo->currentText();
        object.insert("foreground", exp.match(text).captured(0));

        json = QJsonDocument(object).toJson();
    }

    QProcess process;
    process.startDetached(
         "./objectwheel-themer",
         QStringList() << "capture"
                       << (m_version == V1 ? "v1" : "v2")
                       << (m_version == V1 ? m_stylesCombo->currentText() : json)
                       << tmpFile.fileName()
    );

    process.waitForStarted();

    Delayer::delay(2000);

    QPixmap preview(tmpFile.fileName());
    preview.setDevicePixelRatio(DPR);
    m_previewPicture->setPixmap(preview);
}

namespace {
    const QStringList STYLES = {
        QObject::tr("Base"),
        QObject::tr("Flat"),
        QObject::tr("Desktop")
    };

    const QStringList STYLES_V2 = {
        QObject::tr("Default"),
        QObject::tr("Fusion"),
        QObject::tr("Imagine"),
        QObject::tr("Material"),
        QObject::tr("Universal")
    };

    const QStringList MATERIAL_THEMES = {
        QObject::tr("Light"),
        QObject::tr("Dark")
    };

    const QStringList UNIVERSAL_THEMES = {
        QObject::tr("Light"),
        QObject::tr("Dark")
    };

    const QStringList MATERIAL_COLORS_LIGHT = {
        QObject::tr("Red (#F44336)"),
        QObject::tr("Pink (#E91E63)"),
        QObject::tr("Purple (#9C27B0)"),
        QObject::tr("DeepPurple (#673AB7)"),
        QObject::tr("Indigo (#3F51B5)"),
        QObject::tr("Blue (#2196F3)"),
        QObject::tr("LightBlue (#03A9F4)"),
        QObject::tr("Cyan (#00BCD4)"),
        QObject::tr("Teal (#009688)"),
        QObject::tr("Green (#4CAF50)"),
        QObject::tr("LightGreen (#8BC34A)"),
        QObject::tr("Lime (#CDDC39)"),
        QObject::tr("Yellow (#FFEB3B)"),
        QObject::tr("Amber (#FFC107)"),
        QObject::tr("Orange (#FF9800)"),
        QObject::tr("DeepOrange (#FF5722)"),
        QObject::tr("Brown (#795548)"),
        QObject::tr("Grey (#9E9E9E)"),
        QObject::tr("BlueGrey (#607D8B)"),
        QObject::tr("Dark (#dd000000)"),
        QObject::tr("Light (#fafafa)")
    };

    const QStringList MATERIAL_COLORS_DARK = {
        QObject::tr("Red (#EF9A9A)"),
        QObject::tr("Pink (#F48FB1)"),
        QObject::tr("Purple (#CE93D8)"),
        QObject::tr("DeepPurple (#B39DDB)"),
        QObject::tr("Indigo (#9FA8DA)"),
        QObject::tr("Blue (#90CAF9)"),
        QObject::tr("LightBlue (#81D4FA)"),
        QObject::tr("Cyan (#80DEEA)"),
        QObject::tr("Teal (#80CBC4)"),
        QObject::tr("Green (#A5D6A7)"),
        QObject::tr("LightGreen (#C5E1A5)"),
        QObject::tr("Lime (#E6EE9C)"),
        QObject::tr("Yellow (#FFF59D)"),
        QObject::tr("Amber (#FFE082)"),
        QObject::tr("Orange (#FFCC80)"),
        QObject::tr("DeepOrange (#FFAB91)"),
        QObject::tr("Brown (#BCAAA4)"),
        QObject::tr("Grey (#EEEEEE)"),
        QObject::tr("BlueGrey (#B0BEC5)"),
        QObject::tr("Light (#ffffff)"),
        QObject::tr("Dark (#303030)")
    };

    const QStringList UNIVERSAL_COLORS = {
        QObject::tr("Lime (#A4C400)"),
        QObject::tr("Green (#60A917)"),
        QObject::tr("Emerald (#008A00)"),
        QObject::tr("Teal (#00ABA9)"),
        QObject::tr("Cyan (#1BA1E2)"),
        QObject::tr("Cobalt (#3E65FF)"),
        QObject::tr("Indigo (#6A00FF)"),
        QObject::tr("Violet (#AA00FF)"),
        QObject::tr("Pink (#F472D0)"),
        QObject::tr("Magenta (#D80073)"),
        QObject::tr("Crimson (#A20025)"),
        QObject::tr("Red (#E51400)"),
        QObject::tr("Orange (#FA6800)"),
        QObject::tr("Amber (#F0A30A)"),
        QObject::tr("Yellow (#E3C800)"),
        QObject::tr("Brown (#825A2C)"),
        QObject::tr("Olive (#6D8764)"),
        QObject::tr("Steel (#647687)"),
        QObject::tr("Mauve (#76608A)"),
        QObject::tr("Taupe (#87794E)"),
        QObject::tr("Dark (#000000)"),
        QObject::tr("White (#FFFFFF)")
    };
}