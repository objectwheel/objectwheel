#include <resetverificationwidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
#include <apimanager.h>
#include <countdown.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <servermanager.h>

#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>

enum Fields { Code, Password, ConfirmPassword };
enum Buttons { CompletePasswordReset, ResendPasswordResetCode, Cancel };

ResetVerificationWidget::ResetVerificationWidget(QWidget* parent) : QWidget(parent)
  , m_countdown(new Countdown(this))
  , m_emailLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/password.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto resetLabel = new QLabel(this);
    resetLabel->setFont(f);
    resetLabel->setText(tr("Password Reset Verification"));

    auto countdownLabel = new QLabel(this);
    countdownLabel->setAlignment(Qt::AlignHCenter);
    countdownLabel->setText(tr("You have left"));
    countdownLabel->setStyleSheet(QStringLiteral("color: #77000000"));

    m_emailLabel->setAlignment(Qt::AlignHCenter);
    m_emailLabel->setStyleSheet(QStringLiteral("color: #77000000"));

    m_bulkEdit->add(Code, tr("Verification Code"));
    m_bulkEdit->add(Password, tr("New Password"));
    m_bulkEdit->add(ConfirmPassword, tr("Confirm New Password"));

    m_bulkEdit->get<QLineEdit*>(Code)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Code)->setValidator(
                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("\\d{1,6}")), this));
    m_bulkEdit->get<QLineEdit*>(Password)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Password)->setEchoMode(QLineEdit::Password);
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->setEchoMode(QLineEdit::Password);

    m_buttons->add(Cancel, QLatin1String("#CC5D67"), QLatin1String("#B2525A"));
    m_buttons->add(ResendPasswordResetCode, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(CompletePasswordReset, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    m_buttons->get(Cancel)->setText(tr("Cancel"));
    m_buttons->get(ResendPasswordResetCode)->setText(tr("Resend"));
    m_buttons->get(CompletePasswordReset)->setText(tr("Verify"));
    m_buttons->get(Cancel)->setIcon(QIcon(QStringLiteral(":/images/welcome/cancel.png")));
    m_buttons->get(ResendPasswordResetCode)->setIcon(QIcon(QStringLiteral(":/images/welcome/reset.png")));
    m_buttons->get(CompletePasswordReset)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    m_buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(ResendPasswordResetCode)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(CompletePasswordReset)->setCursor(Qt::PointingHandCursor);
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
    layout->addWidget(resetLabel, 0, Qt::AlignHCenter);
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
            this, &ResetVerificationWidget::onCancelClicked);
    connect(m_buttons->get(CompletePasswordReset), &QPushButton::clicked,
            this, &ResetVerificationWidget::onCompletePasswordResetClicked);
    connect(m_bulkEdit, &BulkEdit::returnPressed,
            this, &ResetVerificationWidget::onCompletePasswordResetClicked);
    connect(m_buttons->get(ResendPasswordResetCode), &QPushButton::clicked,
            this, &ResetVerificationWidget::onResendPasswordResetCodeClicked);
    connect(ApiManager::instance(), &ApiManager::completePasswordResetSuccessful,
            this, &ResetVerificationWidget::onCompletePasswordResetSuccessful);
    connect(ApiManager::instance(), &ApiManager::completePasswordResetFailure,
            this, &ResetVerificationWidget::onCompletePasswordResetFailure);
    connect(ApiManager::instance(), &ApiManager::resendPasswordResetCodeSuccessful,
            this, &ResetVerificationWidget::onResendPasswordResetCodeSuccessful);
    connect(ApiManager::instance(), &ApiManager::resendPasswordResetCodeFailure,
            this, &ResetVerificationWidget::onResendPasswordResetCodeFailure);
    connect(m_countdown, &Countdown::finished,
            this, &ResetVerificationWidget::onCountdownFinished);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &ResetVerificationWidget::onServerDisconnected);
}

void ResetVerificationWidget::setEmail(const QString& email)
{
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));
    m_buttons->get(ResendPasswordResetCode)->setEnabled(true);
    m_bulkEdit->get<QLineEdit*>(Code)->clear();
    m_bulkEdit->get<QLineEdit*>(Password)->clear();
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->clear();
    m_emailLabel->setText(tr("Please use the verification code that we have\n"
                             "sent to the email address below to reset your password\n") + email);
    m_countdown->start(300); // 5 mins
}

void ResetVerificationWidget::onCancelClicked()
{
    m_countdown->stop();
    emit cancel();
}

void ResetVerificationWidget::onCompletePasswordResetClicked()
{
    const QString email = m_emailLabel->text().split(QLatin1Char('\n')).at(2);
    const QString& code = m_bulkEdit->get<QLineEdit*>(Code)->text();
    const QString& password = m_bulkEdit->get<QLineEdit*>(Password)->text();
    const QString& cpassword = m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->text();
    const QString& hash = UtilityFunctions::hashPassword(password);
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));

    if (m_countdown->hasExpired()) {
        UtilityFunctions::showMessage(this,
                                      tr("Verification code expired"),
                                      tr("Please try again later."),
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

    if (password.isEmpty() || cpassword.isEmpty()) {
        UtilityFunctions::showMessage(this,
                                      tr("Fields cannot be left blank"),
                                      tr("Please fill in all the required fields."),
                                      QMessageBox::Information);
        return;
    }

    if (password.size() > 255 || cpassword.size() > 255) {
        UtilityFunctions::showMessage(this,
                                      tr("Password too long"),
                                      tr("No passwords can be larger than 255 characters."),
                                      QMessageBox::Information);
        return;
    }

    if (password != cpassword) {
        UtilityFunctions::showMessage(this,
                                      tr("Incorrect passwords"),
                                      tr("Passwords you entered do not match."),
                                      QMessageBox::Information);
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt password"),
                                      tr("Your password must comply with following standards:\n"
                                         "•  Length must be between 6 and 35 characters\n"
                                         "•  Only Latin-1 characters are allowed\n"
                                         "•  Whitespace characters are not allowed\n"
                                         "•  It can contain a-z, A-Z, 0-9\n"
                                         "•  It can also contain following special characters:\n"
                                         "   [ ] > < { } * ! @ - # $ % ^ & + = ~ . , :"),
                                      QMessageBox::Information);
        return;
    }

    if (ServerManager::isConnected()) {
        m_busyIndicator->start();
        ApiManager::completePasswordReset(email, hash, code);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void ResetVerificationWidget::onResendPasswordResetCodeClicked()
{
    const QString email = m_emailLabel->text().split(QLatin1Char('\n')).at(2);
    Q_ASSERT(UtilityFunctions::isEmailFormatCorrect(email));

    if (m_countdown->hasExpired()) {
        UtilityFunctions::showMessage(this,
                                      tr("Verification code expired"),
                                      tr("Please try again later."),
                                      QMessageBox::Information);
        return;
    }

    if (ServerManager::isConnected()) {
        m_buttons->get(ResendPasswordResetCode)->setEnabled(false);
        m_busyIndicator->start();
        ApiManager::resendPasswordResetCode(email);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void ResetVerificationWidget::onCompletePasswordResetSuccessful()
{
    m_busyIndicator->stop();
    m_countdown->stop();
    emit done();
}

void ResetVerificationWidget::onCompletePasswordResetFailure()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Invalid information entered"),
                                  tr("Incorrect verification code entered or maybe you have "
                                     "tried it too much. Please try again some time later."));
}

void ResetVerificationWidget::onResendPasswordResetCodeSuccessful()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Resend succeed"),
                                  tr("New verification code has been sent."),
                                  QMessageBox::Information);
}

void ResetVerificationWidget::onResendPasswordResetCodeFailure()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("The server rejected your request"),
                                  tr("You might have exceeded the verification "
                                     "trial limit. Please try again some time later."));

}

void ResetVerificationWidget::onCountdownFinished()
{
    UtilityFunctions::showMessage(this,
                                  tr("Verification code expired"),
                                  tr("Please try again later."),
                                  QMessageBox::Information);
}

void ResetVerificationWidget::onServerDisconnected()
{
    if (m_busyIndicator->isSpinning()) {
        m_busyIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}
