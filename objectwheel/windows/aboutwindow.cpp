#include <aboutwindow.h>
#include <coreconstants.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QVBoxLayout>
#include <QPushButton>
#include <QDate>

AboutWindow::AboutWindow(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_logoLabel(new QLabel(this))
  , m_versionLabel(new QLabel(this))
  , m_okButton(new QPushButton(this))
  , m_legalLabel(new QLabel(this))
{
    setWindowTitle(QStringLiteral(APP_NAME) + QStringLiteral(" (Beta)"));
    setAutoFillBackground(true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                   | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    m_layout->addStretch();
    m_layout->addWidget(m_logoLabel, 0, Qt::AlignCenter);
    m_layout->addWidget(m_versionLabel, 0, Qt::AlignCenter);
    m_layout->addStretch();
    m_layout->addWidget(m_okButton, 0, Qt::AlignCenter);
    m_layout->addStretch();
    m_layout->addWidget(m_legalLabel, 0, Qt::AlignCenter);
    m_layout->setSpacing(20);

    m_logoLabel->setFixedSize(QSize(160, 80));
    m_logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/logo.svg"), QSize(160, 80), this));

    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setText(QStringLiteral("<p>Contact<br><a href='mailto:%4'>%4</a></p>"
                                           "<p>Objectwheel Forum<br><a href='%5'>%5</a></p>"
                                           "<p><b>version</b> v%1 <b>revision</b> %2 <b>date</b><br>%3</p>")
                            .arg(QStringLiteral(APP_VER)).arg(QStringLiteral(APP_GITHASH))
                            .arg(QDateTime::fromString(APP_GITDATE, Qt::ISODate).toString(Qt::SystemLocaleLongDate))
                            .arg(CoreConstants::EMAIL_ADDRESS)
                            .arg(CoreConstants::FORUM_URL));

    m_okButton->setDefault(true);
    m_okButton->setCursor(Qt::PointingHandCursor);
    m_okButton->setText(tr("Close"));
    m_okButton->setFixedWidth(100);
    m_legalLabel->setText(QStringLiteral("<p><b>© 2015 - %1 %2, Inc. All Rights Reserved.</b></p>")
                          .arg(QDate::currentDate().year()).arg(QStringLiteral(APP_CORP)));

    resize(sizeHint());
    move(UtilityFunctions::centerPos(size()));

    connect(m_okButton, &QPushButton::clicked, this, &AboutWindow::done);
}

QSize AboutWindow::sizeHint() const
{
    return {700, 400};
}


