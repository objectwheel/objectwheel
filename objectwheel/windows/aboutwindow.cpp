#include <aboutwindow.h>
#include <appconstants.h>
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
    setWindowTitle(AppConstants::LABEL);
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
    m_versionLabel->setOpenExternalLinks(true);
    m_versionLabel->setText(QStringLiteral("<p>Contact<br><a href='mailto:%1'>%1</a></p>"
                                           "<p>%2 Forum<br><a href='%3'>%3</a></p>"
                                           "<p><b>version</b> v%4 <b>revision</b> %5 <b>date</b><br>%6</p>")
                            .arg(AppConstants::SUPPORT_EMAIL)
                            .arg(AppConstants::NAME)
                            .arg(AppConstants::FORUM_URL)
                            .arg(AppConstants::VERSION)
                            .arg(AppConstants::REVISION)
                            .arg(QDateTime::fromString(AppConstants::BUILD_DATE, Qt::ISODate).toString(Qt::SystemLocaleLongDate)));

    m_okButton->setDefault(true);
    m_okButton->setCursor(Qt::PointingHandCursor);
    m_okButton->setText(tr("Close"));
    m_okButton->setFixedWidth(100);
    m_legalLabel->setText(QStringLiteral("<p><b>© 2015 - %1 %2 All Rights Reserved.</b></p>")
                          .arg(QDate::currentDate().year()).arg(AppConstants::COMPANY_FULL));

    resize(sizeHint());
    move(UtilityFunctions::centerPos(size()));

    connect(m_okButton, &QPushButton::clicked, this, &AboutWindow::done);
}

QSize AboutWindow::sizeHint() const
{
    return {700, 400};
}


