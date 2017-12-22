#include <aboutwindow.h>
#include <fit.h>
#include <global.h>

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QPushButton>

#define pS (QApplication::primaryScreen())

AboutWindow::AboutWindow(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    iconLayout = new QHBoxLayout;
    topLabel = new QLabel;
    iconLabel = new QLabel;
    titleLabel = new QLabel;
    legalLabel = new QLabel;
    okButton = new QPushButton;

    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setAutoFillBackground(true);
    setPalette(p);

    okButton->setText("Ok");
    okButton->setFixedWidth(fit::fx(100));
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked(bool)), SIGNAL(done()));

    QPixmap pixmap(":/resources/images/logo.png");
    pixmap.setDevicePixelRatio(pS->devicePixelRatio());

    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel->setFixedSize(fit::fx(150), fit::fx(74.5));
    iconLabel->setPixmap(
        pixmap.scaled(
            fit::fx(150) * pS->devicePixelRatio(),
            fit::fx(74.5) * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    iconLayout->addStretch();
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch();

    QFont font;
    font.setWeight(QFont::ExtraLight);
    font.setPixelSize(fit::fx(24));

    topLabel->setFont(font);
    topLabel->setText("About Objectwheel");
    topLabel->setStyleSheet("background:transparent; color:#2e3a41;");
    topLabel->setAlignment(Qt::AlignCenter);

    titleLabel->setStyleSheet("background:transparent;color:#2e3a41;");
    titleLabel->setText(TEXT_VERSION);
    titleLabel->setAlignment(Qt::AlignCenter);

    legalLabel->setStyleSheet("background:transparent;color:#2e3a41;");
    legalLabel->setText(TEXT_LEGAL);
    legalLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(topLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(iconLayout);
    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(okButton);
    mainLayout->addStretch();
    mainLayout->addWidget(legalLabel);
    mainLayout->setAlignment(okButton, Qt::AlignCenter);

    setWindowFlags(
        Qt::Dialog |
        Qt::WindowTitleHint |
        Qt::WindowSystemMenuHint |
        Qt::WindowCloseButtonHint |
        Qt::CustomizeWindowHint
    );
}

