#include <aboutwindow.h>
#include <appconstants.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QBoxLayout>
#include <QPushButton>
#include <QDateTime>
#include <QLabel>

AboutWindow::AboutWindow(QWidget* parent) : QWidget(parent)
{
    resize(sizeHint()); // Don't use adjustSize() on Windows
    move(UtilityFunctions::centerPos(size()));
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(AppConstants::LABEL);
    setWindowFlags(Qt::Dialog
                   | Qt::WindowTitleHint
                   | Qt::WindowSystemMenuHint
                   | Qt::WindowCloseButtonHint
                   | Qt::CustomizeWindowHint);

    auto logoLabel = new QLabel(this);
    logoLabel->setFixedSize(QSize(160, 80));
    logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/logo.svg"), QSize(160, 80), this));

    auto versionLabel = new QLabel(this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setOpenExternalLinks(true);
    versionLabel->setText(QStringLiteral("<p>Contact<br><a href='mailto:%1'>%1</a></p>"
                                         "<p>%2 Forum<br><a href='%3'>%3</a></p>"
                                         "<p><b>version</b> v%4 <b>revision</b> %5 <b>date</b><br>%6</p>")
                          .arg(AppConstants::SUPPORT_EMAIL)
                          .arg(AppConstants::NAME)
                          .arg(AppConstants::FORUM_URL)
                          .arg(AppConstants::VERSION)
                          .arg(AppConstants::REVISION)
                          .arg(QDateTime::fromString(AppConstants::BUILD_DATE, Qt::ISODate).toString(Qt::SystemLocaleLongDate)));

    auto okButton = new QPushButton(this);
    okButton->setDefault(true);
    okButton->setCursor(Qt::PointingHandCursor);
    okButton->setText(tr("Close"));

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 6, 0, 6);
    layout->addStretch();
    layout->addWidget(logoLabel, 0, Qt::AlignHCenter);
    layout->addWidget(versionLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(30);
    layout->addWidget(okButton, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(new QLabel(QString::fromUtf8("<b>© 2015 - %1 %2. All Rights Reserved.</b>")
                                 .arg(QDate::currentDate().year()).arg(AppConstants::COMPANY_FULL), this),
                      0, Qt::AlignHCenter);

    connect(okButton, &QPushButton::clicked, this, &AboutWindow::done);
}

QSize AboutWindow::sizeHint() const
{
    return QSize(700, 400);
}
