#include <verificationwidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <countdown.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpressionValidator>

// TODO: Check out if the given email address is valid before sending request to the server

enum Fields { Code };
enum Buttons { Verify, Resend, Cancel };

VerificationWidget::VerificationWidget(QWidget* parent) : QWidget(parent)
  , m_countdown(new Countdown(this))
  , m_emailLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
  , m_buttons(new ButtonSlice(this))
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
{
    auto iconLabel = new QLabel;
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/verification.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto verificationLabel = new QLabel;
    verificationLabel->setFont(f);
    verificationLabel->setText(tr("Email Verification"));

    auto countdownLabel = new QLabel;
    countdownLabel->setAlignment(Qt::AlignHCenter);
    countdownLabel->setText(tr("You have left:"));
    countdownLabel->setStyleSheet(QStringLiteral("color: #70000000"));

    m_emailLabel->setAlignment(Qt::AlignHCenter);
    m_emailLabel->setStyleSheet(QStringLiteral("color: #70000000"));

    m_bulkEdit->setFixedWidth(300);
    m_bulkEdit->add(Code, tr("Enter Verification Code"));
    m_bulkEdit->get<QLineEdit*>(Code)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Code)->setValidator(
                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{1,6}$")), this));

    m_buttons->add(Cancel, QLatin1String("#CC5D67"), QLatin1String("#B2525A"));
    m_buttons->add(Resend, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(Verify, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    m_buttons->get(Cancel)->setText(tr("Cancel"));
    m_buttons->get(Resend)->setText(tr("Resend"));
    m_buttons->get(Verify)->setText(tr("Verify"));
    m_buttons->get(Cancel)->setIcon(QIcon(QStringLiteral(":/images/welcome/cancel.png")));
    m_buttons->get(Resend)->setIcon(QIcon(QStringLiteral(":/images/welcome/reset.png")));
    m_buttons->get(Verify)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    m_buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Resend)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Verify)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = m_bulkEdit->width() / 3.0;
    m_buttons->triggerSettings();

    m_loadingIndicator->setStyleSheet(QStringLiteral("background: transparent"));
    m_loadingIndicator->setColor(palette().text().color());
    m_loadingIndicator->setRoundness(50);
    m_loadingIndicator->setMinimumTrailOpacity(5);
    m_loadingIndicator->setTrailFadePercentage(100);
    m_loadingIndicator->setRevolutionsPerSecond(2);
    m_loadingIndicator->setNumberOfLines(12);
    m_loadingIndicator->setLineLength(5);
    m_loadingIndicator->setInnerRadius(4);
    m_loadingIndicator->setLineWidth(2);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(verificationLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(40);
    layout->addWidget(countdownLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_countdown, 0, Qt::AlignHCenter);
    layout->addSpacing(40);
    layout->addWidget(m_emailLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_loadingIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(m_buttons->get(Verify), &QPushButton::clicked,
            this, &VerificationWidget::onVerifyClicked);
    connect(m_buttons->get(Resend), &QPushButton::clicked,
            this, &VerificationWidget::onResendClicked);
    connect(m_buttons->get(Cancel), &QPushButton::clicked,
            this, &VerificationWidget::onCancelClicked);
    connect(m_countdown, &Countdown::finished,
            this, &VerificationWidget::onCountdownFinished);
}

void VerificationWidget::setEmail(const QString& email)
{
//    resent = false;
    unlock();
    m_countdown->start(300); // 5 mins
    m_emailLabel->setText(tr("Please use the verification code\n"
                             "we have sent to the following email address.\n") + email);
}

void VerificationWidget::clear()
{
    m_countdown->stop();
    m_bulkEdit->get<QLineEdit*>(Code)->clear();
}

void VerificationWidget::lock()
{
    m_bulkEdit->setDisabled(true);
    m_buttons->get(Verify)->setDisabled(true);
    m_buttons->get(Resend)->setDisabled(true);
    m_buttons->get(Cancel)->setDisabled(true);
    m_loadingIndicator->start();
}

void VerificationWidget::unlock()
{
    m_bulkEdit->setEnabled(true);
    m_buttons->get(Verify)->setEnabled(true);
//    if (!resent)
        m_buttons->get(Resend)->setEnabled(true);
    m_buttons->get(Cancel)->setEnabled(true);
    m_loadingIndicator->stop();
}

void VerificationWidget::onCancelClicked()
{
    clear();
    emit cancel();
}

void VerificationWidget::onResendClicked()
{
    auto email = m_emailLabel->text().split("\n").at(2);

    lock();

    //    bool succeed = RegistrationApiManager::resend(email);

    //    if (succeed) {
    //        clear();
    //        m_countdown->start();

    //        QMessageBox::information(
    //            this,
    //            tr("Resend succeed"),
    //            tr("New verification code has been sent.")
    //        );
    //    } else {
    //        QMessageBox::warning(
    //            this,
    //            tr("Oops"),
    //            tr("Server rejected your request. You might exceed "
    //               "the verification trial limit. Try again later.")
    //        );
    //    }

    //    resent = true;
    //    unlock();
}

void VerificationWidget::onVerifyClicked()
{
    auto email = m_emailLabel->text().split("\n").at(2);
    auto code = m_bulkEdit->get<QLineEdit*>(Code)->text();

    if (code.isEmpty() || code.size() != 6) {
        UtilityFunctions::showMessage(this, tr("Incorrect information"),
                                      tr("Verification code is incorrect."));
        return;
    }

    lock();

    //    bool succeed = RegistrationApiManager::verify(email, code);

    //    if (succeed)
    //        clear();
    //    else {
    //        QMessageBox::warning(
    //            this,
    //            tr("Oops"),
    //            tr("Server rejected your code. Or, you might exceed "
    //               "the verification trial limit. Try again later.")
    //        );
    //    }

    //    unlock();

    //    if (succeed)
    //        emit done();
}

void VerificationWidget::onVerifySuccessful()
{

}

void VerificationWidget::onVerifyFailure()
{

}

void VerificationWidget::onResendSuccessful()
{

}

void VerificationWidget::onResendFailure()
{

}

void VerificationWidget::onDisconnected()
{

}

void VerificationWidget::onCountdownFinished()
{
    UtilityFunctions::showMessage(this,
                                  tr("Expired"),
                                  tr("Your verification code has been expired, please try again later."));
}
