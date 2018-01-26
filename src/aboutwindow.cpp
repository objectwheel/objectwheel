#include <aboutwindow.h>
#include <fit.h>
#include <global.h>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QScreen>
#include <QPushButton>

#define pS        (QApplication::primaryScreen())
#define PATH_LOGO (":/resources/images/logo.png")
#define SIZE_LOGO (QSize(fit::fx(160), fit::fx(80)))

AboutWindow::AboutWindow(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _titleLabel = new QLabel;
    _logoLabel = new QLabel;
    _versionLabel = new QLabel;
    _okButton = new QPushButton;
    _legalLabel = new QLabel;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");

    setPalette(p);
    setWindowTitle(APP_NAME);
    setAutoFillBackground(true);
    setWindowFlags(
        Qt::Dialog |
        Qt::WindowTitleHint |
        Qt::WindowSystemMenuHint |
        Qt::WindowCloseButtonHint |
        Qt::CustomizeWindowHint
    );

    _layout->addWidget(_titleLabel);
    _layout->addStretch();
    _layout->addWidget(_logoLabel);
    _layout->addWidget(_versionLabel);
    _layout->addStretch();
    _layout->addWidget(_okButton);
    _layout->addStretch();
    _layout->addWidget(_legalLabel);
    _layout->setAlignment(_titleLabel, Qt::AlignCenter);
    _layout->setAlignment(_logoLabel, Qt::AlignCenter);
    _layout->setAlignment(_versionLabel, Qt::AlignCenter);
    _layout->setAlignment(_okButton, Qt::AlignCenter);
    _layout->setAlignment(_legalLabel, Qt::AlignCenter);
    _layout->setSpacing(fit::fx(20));

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(fit::fx(24));

    _titleLabel->setFont(f);
    _titleLabel->setText("About Objectwheel");
    _titleLabel->setStyleSheet("color:#2e3a41;");

    QPixmap px(PATH_LOGO);
    px.setDevicePixelRatio(pS->devicePixelRatio());

    _logoLabel->setFixedSize(SIZE_LOGO);
    _logoLabel->setPixmap(
        px.scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    _versionLabel->setText(TEXT_VERSION);
    _versionLabel->setStyleSheet("color:#2e3a41;");
    _versionLabel->setAlignment(Qt::AlignCenter);

    _okButton->setText("Ok");
    _okButton->setFixedWidth(fit::fx(100));
    _okButton->setCursor(Qt::PointingHandCursor);
    connect(_okButton, SIGNAL(clicked(bool)), SIGNAL(done()));

    _legalLabel->setText(TEXT_LEGAL);
    _legalLabel->setStyleSheet("color:#2e3a41;");
}

