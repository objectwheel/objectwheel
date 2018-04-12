#include <themechooserwidget.h>
#include <dpr.h>
#include <fit.h>
#include <delayer.h>
#include <saveutils.h>
#include <projectbackend.h>
#include <filemanager.h>

#include <QtQuickControls2>
#include <QtWidgets>

namespace {
    QPalette clearPalette;
    extern const QStringList STYLES;
    extern const QStringList STYLES_V2;
    extern const QStringList MATERIAL_THEMES;
    extern const QStringList UNIVERSAL_THEMES;
    extern const QStringList MATERIAL_COLORS;
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
    m_stylesLabel->setStyleSheet("color: black");
    m_stylesLabel->setFont(sectionsFont);

    m_stylesCombo->addItems(m_version == V1 ? STYLES : STYLES_V2);
    m_stylesCombo->setFixedWidth(fit::fx(150));

    m_seeRunningButton->setText(tr("See Running"));
    m_seeRunningButton->setDefault(true);

    if (m_version == V2) {
        m_themesLabel->setText(tr("Theme:"));
        m_themesLabel->setStyleSheet("color: black");
        m_themesLabel->setFont(sectionsFont);

        m_themesCombo->setCurrentText("None");
        m_themesCombo->setFixedWidth(fit::fx(150));

        m_detailsLabel->setText(tr("Details:"));
        m_detailsLabel->setStyleSheet("color: black");
        m_detailsLabel->setFont(sectionsFont);

        m_accentDetailLabel->setText(tr("Accent color:"));
        m_accentDetailLabel->setStyleSheet("color: black");

        m_primaryDetailLabel->setText(tr("Primary color:"));
        m_primaryDetailLabel->setStyleSheet("color: black");

        m_foregroundDetailLabel->setText(tr("Foreground color:"));
        m_foregroundDetailLabel->setStyleSheet("color: black");

        m_backgroundDetailLabel->setText(tr("Background color:"));
        m_backgroundDetailLabel->setStyleSheet("color: black");

        m_accentColorsCombo->setFixedWidth(fit::fx(200));
        m_primaryColorsCombo->setFixedWidth(fit::fx(200));
        m_foregroundColorsCombo->setFixedWidth(fit::fx(200));
        m_backgroundColorsCombo->setFixedWidth(fit::fx(200));

        m_accentColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_accentColorLabel->setFrameShape(QLabel::StyledPanel);
        m_accentColorLabel->setFrameShadow(QLabel::Plain);
        m_accentColorLabel->setAutoFillBackground(true);

        m_primaryColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_primaryColorLabel->setFrameShape(QLabel::StyledPanel);
        m_primaryColorLabel->setFrameShadow(QLabel::Plain);
        m_primaryColorLabel->setAutoFillBackground(true);

        m_foregroundColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_foregroundColorLabel->setFrameShape(QLabel::StyledPanel);
        m_foregroundColorLabel->setFrameShadow(QLabel::Plain);
        m_foregroundColorLabel->setAutoFillBackground(true);

        m_backgroundColorLabel->setFixedSize(fit::fx(QSizeF(20, 20)).toSize());
        m_backgroundColorLabel->setFrameShape(QLabel::StyledPanel);
        m_backgroundColorLabel->setFrameShadow(QLabel::Plain);
        m_backgroundColorLabel->setAutoFillBackground(true);

        clearPalette = m_backgroundColorLabel->palette();
        clearPalette.setColor(m_backgroundColorLabel->backgroundRole(), Qt::transparent);

        m_accentColorButton->setText("...");
        m_primaryColorButton->setText("...");
        m_foregroundColorButton->setText("...");
        m_backgroundColorButton->setText("...");

        m_customizationLabel->setText(tr("Customization:"));
        m_customizationLabel->setStyleSheet("color: black");
        m_customizationLabel->setFont(sectionsFont);

        m_customizationPicture->setScaledContents(false);
        m_customizationPicture->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    }

    m_previewLabel->setText(tr("Preview:"));
    m_previewLabel->setStyleSheet("color: black");
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
        connect(m_accentColorsCombo, SIGNAL(activated(int)), SLOT(saveTheme()), Qt::QueuedConnection);
        connect(m_primaryColorsCombo, SIGNAL(activated(int)), SLOT(saveTheme()), Qt::QueuedConnection);
        connect(m_foregroundColorsCombo, SIGNAL(activated(int)), SLOT(saveTheme()), Qt::QueuedConnection);
        connect(m_backgroundColorsCombo, SIGNAL(activated(int)), SLOT(saveTheme()), Qt::QueuedConnection);
        connect(m_themesCombo, SIGNAL(activated(int)), SLOT(saveTheme()), Qt::QueuedConnection);

        connect(m_accentColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_primaryColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_foregroundColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_backgroundColorsCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
        connect(m_themesCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);
    }

    connect(m_stylesCombo, SIGNAL(activated(int)), SLOT(saveTheme()), Qt::QueuedConnection);
    connect(m_stylesCombo, SIGNAL(activated(int)), SLOT(refresh()), Qt::QueuedConnection);

    if (m_version == V2) {
        connect(m_accentColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_accentColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty())
                m_accentColorLabel->setPalette(clearPalette);
            else {
                auto p = clearPalette;
                p.setColor(m_accentColorLabel->backgroundRole(), colorName);
                m_accentColorLabel->setPalette(p);
            }
        });

        connect(m_primaryColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_primaryColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty())
                m_primaryColorLabel->setPalette(clearPalette);
            else {
                auto p = clearPalette;
                p.setColor(m_primaryColorLabel->backgroundRole(), colorName);
                m_primaryColorLabel->setPalette(p);
            }
        });

        connect(m_foregroundColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_foregroundColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty())
                m_foregroundColorLabel->setPalette(clearPalette);
            else {
                auto p = clearPalette;
                p.setColor(m_foregroundColorLabel->backgroundRole(), colorName);
                m_foregroundColorLabel->setPalette(p);
            }
        });

        connect(m_backgroundColorsCombo, &QComboBox::currentTextChanged, [=] {
            const auto& text = m_backgroundColorsCombo->currentText();

            QRegularExpression exp("#.*(?=\\))");
            auto colorName = exp.match(text).captured(0);

            if (colorName.isEmpty())
                m_backgroundColorLabel->setPalette(clearPalette);
            else {
                auto p = clearPalette;
                p.setColor(m_backgroundColorLabel->backgroundRole(), colorName);
                m_backgroundColorLabel->setPalette(p);
            }
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
                m_accentColorsCombo->addItem(tr("Theme default"));
                m_primaryColorsCombo->addItem(tr("Theme default"));
                m_foregroundColorsCombo->addItem(tr("Theme default"));
                m_backgroundColorsCombo->addItem(tr("Theme default"));

                for (int i = 0; i < MATERIAL_COLORS.size(); i++) {
                    const auto& text = MATERIAL_COLORS[i];
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
            } else if (m_stylesCombo->currentText() == "Universal") {
                m_primaryColorsCombo->addItem(tr("Unavailable"));
                m_primaryColorsCombo->setDisabled(true);
                m_primaryColorButton->setDisabled(true);

                m_accentColorsCombo->addItem(tr("Theme default"));
                m_foregroundColorsCombo->addItem(tr("Theme default"));
                m_backgroundColorsCombo->addItem(tr("Theme default"));

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

    connect(m_seeRunningButton, SIGNAL(clicked(bool)), SLOT(run()));
    resize(1299, 1299);
}

void ThemeChooserWidget::reset()
{
    const auto& object = SaveUtils::theme(ProjectBackend::instance()->dir()).toObject();
    const auto& version = object.value("version").toString();
    const auto& style = object.value("style").toString();

    if (version == "v1") {
        qputenv("QT_QUICK_CONTROLS_1_STYLE", style.toUtf8().constData());
    } else {
        const auto& theme = object.value("theme").toString();
        const auto& accent = object.value("accent").toString();
        const auto& primary = object.value("primary").toString();
        const auto& foreground = object.value("foreground").toString();
        const auto& background = object.value("background").toString();

        qputenv("QT_QUICK_CONTROLS_STYLE", style.toUtf8().constData());

        if (style == QString("Material")) {
            if (!theme.isEmpty())
                qputenv("QT_QUICK_CONTROLS_MATERIAL_THEME", theme.toUtf8().constData());

            if (!accent.isEmpty())
                qputenv("QT_QUICK_CONTROLS_MATERIAL_ACCENT", accent.toUtf8().constData());

            if (!primary.isEmpty())
                qputenv("QT_QUICK_CONTROLS_MATERIAL_PRIMARY", primary.toUtf8().constData());

            if (!foreground.isEmpty())
                qputenv("QT_QUICK_CONTROLS_MATERIAL_FOREGROUND", foreground.toUtf8().constData());

            if (!background.isEmpty())
                qputenv("QT_QUICK_CONTROLS_MATERIAL_BACKGROUND", background.toUtf8().constData());
        } else if (style == QString("Universal")) {
            if (!theme.isEmpty())
                qputenv("QT_QUICK_CONTROLS_UNIVERSAL_THEME", theme.toUtf8().constData());

            if (!accent.isEmpty())
                qputenv("QT_QUICK_CONTROLS_UNIVERSAL_ACCENT", accent.toUtf8().constData());

            if (!foreground.isEmpty())
                qputenv("QT_QUICK_CONTROLS_UNIVERSAL_FOREGROUND", foreground.toUtf8().constData());

            if (!background.isEmpty())
                qputenv("QT_QUICK_CONTROLS_UNIVERSAL_BACKGROUND", background.toUtf8().constData());
        }
    }
}

void ThemeChooserWidget::run()
{
    QProcess::startDetached(
         qApp->applicationDirPath() + "/objectwheel-themer",
         QStringList() << "show"
                       << ProjectBackend::instance()->dir()
    );
}

void ThemeChooserWidget::refresh()
{
    QTemporaryFile tmpFile;
    tmpFile.open();
    tmpFile.close();

    QProcess process;
    process.startDetached(
         qApp->applicationDirPath() + "/objectwheel-themer",
         QStringList() << "capture"
                       << ProjectBackend::instance()->dir()
                       << tmpFile.fileName()
    );

    process.waitForStarted();
    process.waitForFinished();

    QPixmap preview(tmpFile.fileName());
    preview.setDevicePixelRatio(DPR);
    m_previewPicture->setPixmap(preview);
}

void ThemeChooserWidget::saveTheme()
{
    QJsonObject object;
    QRegularExpression exp("#.*(?=\\))");

    QString text = m_version == V2 ? "v2" : "v1";
    object.insert("version", text);

    text = m_stylesCombo->currentText();
    object.insert("style", text);

    if (m_version == V2) {
        text = m_themesCombo->currentText();
        if (!text.contains("default"))
            object.insert("theme", text);

        text = m_accentColorsCombo->currentText();
        if (!text.contains("default"))
            object.insert("accent", exp.match(text).captured(0));

        text = m_primaryColorsCombo->currentText();
        if (!text.contains("default"))
            object.insert("primary", exp.match(text).captured(0));

        text = m_backgroundColorsCombo->currentText();
        if (!text.contains("default"))
            object.insert("background", exp.match(text).captured(0));

        text = m_foregroundColorsCombo->currentText();
        if (!text.contains("default"))
            object.insert("foreground", exp.match(text).captured(0));
    }

    SaveUtils::setProjectProperty(ProjectBackend::instance()->dir(), PTAG_THEME, object);
}

namespace {
    const QStringList STYLES = {
        QObject::tr("Desktop"),
        QObject::tr("Base"),
        QObject::tr("Flat")
    };

    const QStringList STYLES_V2 = {
        QObject::tr("Default"),
        QObject::tr("Fusion"),
        QObject::tr("Imagine"),
        QObject::tr("Material"),
        QObject::tr("Universal")
    };

    const QStringList MATERIAL_THEMES = {
        QObject::tr("System default"),
        QObject::tr("Light"),
        QObject::tr("Dark")
    };

    const QStringList UNIVERSAL_THEMES = {
        QObject::tr("System default"),
        QObject::tr("Light"),
        QObject::tr("Dark")
    };

    const QStringList MATERIAL_COLORS = {
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
        QObject::tr("Dark (#303030)"),
        QObject::tr("Red 2 (#F44336)"),
        QObject::tr("Pink 2 (#E91E63)"),
        QObject::tr("Purple 2 (#9C27B0)"),
        QObject::tr("DeepPurple 2 (#673AB7)"),
        QObject::tr("Indigo 2 (#3F51B5)"),
        QObject::tr("Blue 2 (#2196F3)"),
        QObject::tr("LightBlue 2 (#03A9F4)"),
        QObject::tr("Cyan 2 (#00BCD4)"),
        QObject::tr("Teal 2 (#009688)"),
        QObject::tr("Green 2 (#4CAF50)"),
        QObject::tr("LightGreen 2 (#8BC34A)"),
        QObject::tr("Lime 2 (#CDDC39)"),
        QObject::tr("Yellow 2 (#FFEB3B)"),
        QObject::tr("Amber 2 (#FFC107)"),
        QObject::tr("Orange 2 (#FF9800)"),
        QObject::tr("DeepOrange 2 (#FF5722)"),
        QObject::tr("Brown 2 (#795548)"),
        QObject::tr("Grey 2 (#9E9E9E)"),
        QObject::tr("BlueGrey 2 (#607D8B)"),
        QObject::tr("Dark 2 (#dd000000)"),
        QObject::tr("Light 2 (#fafafa)")
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
