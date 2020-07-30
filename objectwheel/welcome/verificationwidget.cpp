#include <verificationwidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <countdown.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <servermanager.h>

#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>

enum Fields { Code };
enum Buttons { CompleteSignup, ResendSignupCode, Cancel };

VerificationWidget::VerificationWidget(QWidget* parent) : QWidget(parent)
  , m_countdown(new Countdown(this))
  , m_emailLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
  , m_buttons(new ButtonSlice(this))
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/verification.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto verificationLabel = new QLabel(this);
    verificationLabel->setFont(f);
    verificationLabel->setText(tr("Email Verification"));

    auto countdownLabel = new QLabel(this);
    countdownLabel->setAlignment(Qt::AlignHCenter);
    countdownLabel->setText(tr("You have left"));
    countdownLabel->setStyleSheet(QStringLiteral("color: #77000000"));

    m_emailLabel->setAlignment(Qt::AlignHCenter);
    m_emailLabel->setStyleSheet(QStringLiteral("color: #77000000"));

    m_bulkEdit->setFixedWidth(300);
    m_bulkEdit->add(Code, tr("Enter Verification Code"));
    m_bulkEdit->get<QLineEdit*>(Code)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Code)->setValidator(
                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{1,6}$")), this));

    m_buttons->add(Cancel, QLatin1String("#CC5D67"), QLatin1String("#B2525A"));
    m_buttons->add(ResendSignupCode, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(CompleteSignup, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    m_buttons->get(Cancel)->setText(tr("Cancel"));
    m_buttons->get(ResendSignupCode)->setText(tr("Resend"));
    m_buttons->get(CompleteSignup)->setText(tr("Verify"));
    m_buttons->get(Cancel)->setIcon(QIcon(QStringLiteral(":/images/welcome/cancel.png")));
    m_buttons->get(ResendSignupCode)->setIcon(QIcon(QStringLiteral(":/images/welcome/reset.png")));
    m_buttons->get(CompleteSignup)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    m_buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(ResendSignupCode)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(CompleteSignup)->setCursor(Qt::PointingHandCursor);
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

    connect(m_buttons->get(Cancel), &QPushButton::clicked,
            this, &VerificationWidget::onCancelClicked);
    connect(m_buttons->get(CompleteSignup), &QPushButton::clicked,
            this, &VerificationWidget::onCompleteSignupClicked);
    connect(m_buttons->get(ResendSignupCode), &QPushButton::clicked,
            this, &VerificationWidget::onResendSignupCodeClicked);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::completeSignupSuccessful,
            this, &VerificationWidget::onCompleteSignupSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::completeSignupFailure,
            this, &VerificationWidget::onCompleteSignupFailure);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resendSignupCodeSuccessful,
            this, &VerificationWidget::onResendSignupCodeSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resendSignupCodeFailure,
            this, &VerificationWidget::onResendSignupCodeFailure);
    connect(m_countdown, &Countdown::finished,
            this, &VerificationWidget::onCountdownFinished);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &VerificationWidget::onServerDisconnected);
}

void VerificationWidget::setEmail(const QString& email)
{
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));
    m_buttons->get(ResendSignupCode)->setEnabled(true);
    m_bulkEdit->get<QLineEdit*>(Code)->clear();
    m_emailLabel->setText(tr("Please use the verification code that we have\n"
                             "sent to the email address below to complete your registration\n") + email);
    m_countdown->start(300); // 5 mins
}

void VerificationWidget::onCancelClicked()
{
    m_countdown->stop();
    emit cancel();
}

void VerificationWidget::onCompleteSignupClicked()
{
    const QString email = m_emailLabel->text().split(QLatin1Char('\n')).at(2);
    const QString& code = m_bulkEdit->get<QLineEdit*>(Code)->text();
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));

    if (m_countdown->hasExpired()) {
        UtilityFunctions::showMessage(this,
                                      tr("Verification code expired"),
                                      tr("Please sign up again later."),
                                      QMessageBox::Information);
        return;
    }

    if (code.isEmpty() || code.size() != 6) {
        UtilityFunctions::showMessage(this,
                                      tr("Invalid information entered"),
                                      tr("Verification code is not appropriate."),
                                      QMessageBox::Information);
        return;
    }

    if (ServerManager::isConnected()) {
        m_loadingIndicator->start();
        RegistrationApiManager::completeSignup(email, code);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void VerificationWidget::onResendSignupCodeClicked()
{
    const QString email = m_emailLabel->text().split(QLatin1Char('\n')).at(2);
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));

    if (m_countdown->hasExpired()) {
        UtilityFunctions::showMessage(this,
                                      tr("Verification code expired"),
                                      tr("Please sign up again later."),
                                      QMessageBox::Information);
        return;
    }

    if (ServerManager::isConnected()) {
        m_buttons->get(ResendSignupCode)->setEnabled(false);
        m_loadingIndicator->start();
        RegistrationApiManager::resendSignupCode(email);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void VerificationWidget::onCompleteSignupSuccessful()
{
    m_loadingIndicator->stop();
    m_countdown->stop();
    emit done();
}

void VerificationWidget::onCompleteSignupFailure()
{
    m_loadingIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Invalid information entered"),
                                  tr("Incorrect verification code entered or maybe you have "
                                     "tried it too much. Please try again some time later."));
}

void VerificationWidget::onResendSignupCodeSuccessful()
{
    m_loadingIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Resend succeed"),
                                  tr("New verification code has been sent."),
                                  QMessageBox::Information);
}

void VerificationWidget::onResendSignupCodeFailure()
{
    m_loadingIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("The server rejected your request"),
                                  tr("You might have exceeded the verification "
                                     "trial limit. Please try again some time later."));

}

void VerificationWidget::onCountdownFinished()
{
    UtilityFunctions::showMessage(this,
                                  tr("Verification code expired"),
                                  tr("Please sign up again later."),
                                  QMessageBox::Information);
}

void VerificationWidget::onServerDisconnected()
{
    if (m_loadingIndicator->isSpinning()) {
        m_loadingIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}
