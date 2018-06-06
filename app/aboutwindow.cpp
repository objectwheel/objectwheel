#include <aboutwindow.h>
#include <dpr.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

AboutWindow::AboutWindow(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_titleLabel(new QLabel)
  , m_logoLabel(new QLabel)
  , m_versionLabel(new QLabel)
  , m_okButton(new QPushButton)
  , m_legalLabel(new QLabel)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#ececec");

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

    m_layout->addWidget(m_titleLabel);
    m_layout->addStretch();
    m_layout->addWidget(m_logoLabel);
    m_layout->addWidget(m_versionLabel);
    m_layout->addStretch();
    m_layout->addWidget(m_okButton);
    m_layout->addStretch();
    m_layout->addWidget(m_legalLabel);
    m_layout->setAlignment(m_titleLabel, Qt::AlignCenter);
    m_layout->setAlignment(m_logoLabel, Qt::AlignCenter);
    m_layout->setAlignment(m_versionLabel, Qt::AlignCenter);
    m_layout->setAlignment(m_okButton, Qt::AlignCenter);
    m_layout->setAlignment(m_legalLabel, Qt::AlignCenter);
    m_layout->setSpacing(20);

    QFont f;
    f.setWeight(QFont::ExtraLight);
    f.setPixelSize(22);

    m_titleLabel->setFont(f);
    m_titleLabel->setText("About Objectwheel");
    m_titleLabel->setStyleSheet("color: black;");

    QPixmap px(":/images/logo.png");
    px.setDevicePixelRatio(DPR);

    m_logoLabel->setFixedSize(QSize(160, 80));
    m_logoLabel->setPixmap(
        px.scaled(
            QSize(160, 80) * DPR,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    m_versionLabel->setText(
        tr("<p><b>version</b> v%1 <b>revision</b> %2 <b>date</b><br> %3 <br></p>")
        .arg(APP_VER)
        .arg(APP_GITHASH)
        .arg(APP_GITDATE)
    );
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setStyleSheet("color: black;");

    m_okButton->setText("Ok");
    m_okButton->setFixedWidth(100);
    m_okButton->setCursor(Qt::PointingHandCursor);
    connect(m_okButton, SIGNAL(clicked(bool)), SIGNAL(done()));

    m_legalLabel->setText(tr("<p><b>© 2015 - 2018 %1 All Rights Reserved.</b></p>").arg(APP_CORP));
    m_legalLabel->setStyleSheet("color: black;");
}

QSize AboutWindow::sizeHint() const
{
    return {700, 400};
}

