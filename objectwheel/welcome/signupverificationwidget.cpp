#include <signupverificationwidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
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

SignupVerificationWidget::SignupVerificationWidget(QWidget* parent) : QWidget(parent)
  , m_countdown(new Countdown(this))
  , m_emailLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
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
    m_buttons->settings().cellWidth = m_bulkEdit->sizeHint().width() / 3.0;
    m_buttons->triggerSettings();

    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setMinimumTrailOpacity(5);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setNumberOfLines(12);
    m_busyIndicator->setLineLength(5);
    m_busyIndicator->setInnerRadius(4);
    m_busyIndicator->setLineWidth(2);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(verificationLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(25);
    layout->addWidget(countdownLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_countdown, 0, Qt::AlignHCenter);
    layout->addSpacing(25);
    layout->addWidget(m_emailLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(m_buttons->get(Cancel), &QPushButton::clicked,
            this, &SignupVerificationWidget::onCancelClicked);
    connect(m_buttons->get(CompleteSignup), &QPushButton::clicked,
            this, &SignupVerificationWidget::onCompleteSignupClicked);
    connect(m_buttons->get(ResendSignupCode), &QPushButton::clicked,
            this, &SignupVerificationWidget::onResendSignupCodeClicked);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::completeSignupSuccessful,
            this, &SignupVerificationWidget::onCompleteSignupSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::completeSignupFailure,
            this, &SignupVerificationWidget::onCompleteSignupFailure);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resendSignupCodeSuccessful,
            this, &SignupVerificationWidget::onResendSignupCodeSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resendSignupCodeFailure,
            this, &SignupVerificationWidget::onResendSignupCodeFailure);
    connect(m_countdown, &Countdown::finished,
            this, &SignupVerificationWidget::onCountdownFinished);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &SignupVerificationWidget::onServerDisconnected);
}

void SignupVerificationWidget::setEmail(const QString& email)
{
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));
    m_buttons->get(ResendSignupCode)->setEnabled(true);
    m_bulkEdit->get<QLineEdit*>(Code)->clear();
    m_emailLabel->setText(tr("Please use the verification code that we have\n"
                             "sent to the email address below to complete your registration\n") + email);
    m_countdown->start(300); // 5 mins
}

void SignupVerificationWidget::onCancelClicked()
{
    m_countdown->stop();
    emit cancel();
}

void SignupVerificationWidget::onCompleteSignupClicked()
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
        m_busyIndicator->start();
        RegistrationApiManager::completeSignup(email, code);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void SignupVerificationWidget::onResendSignupCodeClicked()
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
        m_busyIndicator->start();
        RegistrationApiManager::resendSignupCode(email);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void SignupVerificationWidget::onCompleteSignupSuccessful()
{
    m_busyIndicator->stop();
    m_countdown->stop();
    emit done();
}

void SignupVerificationWidget::onCompleteSignupFailure()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Invalid information entered"),
                                  tr("Incorrect verification code entered or maybe you have "
                                     "tried it too much. Please try again some time later."));
}

void SignupVerificationWidget::onResendSignupCodeSuccessful()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Resend succeed"),
                                  tr("New verification code has been sent."),
                                  QMessageBox::Information);
}

void SignupVerificationWidget::onResendSignupCodeFailure()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("The server rejected your request"),
                                  tr("You might have exceeded the verification "
                                     "trial limit. Please try again some time later."));

}

void SignupVerificationWidget::onCountdownFinished()
{
    UtilityFunctions::showMessage(this,
                                  tr("Verification code expired"),
                                  tr("Please sign up again later."),
                                  QMessageBox::Information);
}

void SignupVerificationWidget::onServerDisconnected()
{
    if (m_busyIndicator->isSpinning()) {
        m_busyIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}
