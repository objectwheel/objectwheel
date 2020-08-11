#include <creditcardwidget.h>

CreditCardWidget::CreditCardWidget(QWidget* parent) : QWidget(parent)
{
//    auto iconLabel = new QLabel(this);
//    iconLabel->setFixedSize(QSize(60, 60));
//    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/verification.svg"), QSize(60, 60), this));

//    QFont f;
//    f.setWeight(QFont::Light);
//    f.setPixelSize(16);

//    auto verificationLabel = new QLabel(this);
//    verificationLabel->setFont(f);
//    verificationLabel->setText(tr("Email Verification"));

//    auto countdownLabel = new QLabel(this);
//    countdownLabel->setAlignment(Qt::AlignHCenter);
//    countdownLabel->setText(tr("You have left"));
//    countdownLabel->setStyleSheet(QStringLiteral("color: #77000000"));

//    m_emailLabel->setAlignment(Qt::AlignHCenter);
//    m_emailLabel->setStyleSheet(QStringLiteral("color: #77000000"));

//    m_bulkEdit->add(Code, tr("Enter Verification Code"));
//    m_bulkEdit->get<QLineEdit*>(Code)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
//    m_bulkEdit->get<QLineEdit*>(Code)->setValidator(
//                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{1,6}$")), this));

//    m_buttons->add(Cancel, QLatin1String("#CC5D67"), QLatin1String("#B2525A"));
//    m_buttons->add(ResendSignupCode, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
//    m_buttons->add(CompleteSignup, QLatin1String("#86CC63"), QLatin1String("#75B257"));
//    m_buttons->get(Cancel)->setText(tr("Cancel"));
//    m_buttons->get(ResendSignupCode)->setText(tr("Resend"));
//    m_buttons->get(CompleteSignup)->setText(tr("Verify"));
//    m_buttons->get(Cancel)->setIcon(QIcon(QStringLiteral(":/images/welcome/cancel.png")));
//    m_buttons->get(ResendSignupCode)->setIcon(QIcon(QStringLiteral(":/images/welcome/reset.png")));
//    m_buttons->get(CompleteSignup)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
//    m_buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
//    m_buttons->get(ResendSignupCode)->setCursor(Qt::PointingHandCursor);
//    m_buttons->get(CompleteSignup)->setCursor(Qt::PointingHandCursor);
//    m_buttons->settings().cellWidth = m_bulkEdit->sizeHint().width() / 3.0;
//    m_buttons->triggerSettings();

//    m_busyIndicator->setRoundness(50);
//    m_busyIndicator->setMinimumTrailOpacity(5);
//    m_busyIndicator->setTrailFadePercentage(100);
//    m_busyIndicator->setRevolutionsPerSecond(2);
//    m_busyIndicator->setNumberOfLines(12);
//    m_busyIndicator->setLineLength(5);
//    m_busyIndicator->setInnerRadius(4);
//    m_busyIndicator->setLineWidth(2);

//    auto layout = new QVBoxLayout(this);
//    layout->setSpacing(6);
//    layout->setContentsMargins(0, 0, 0, 0);
//    layout->addStretch();
//    layout->addStretch();
//    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
//    layout->addWidget(verificationLabel, 0, Qt::AlignHCenter);
//    layout->addSpacing(25);
//    layout->addWidget(countdownLabel, 0, Qt::AlignHCenter);
//    layout->addWidget(m_countdown, 0, Qt::AlignHCenter);
//    layout->addSpacing(25);
//    layout->addWidget(m_emailLabel, 0, Qt::AlignHCenter);
//    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
//    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
//    layout->addStretch();
//    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
//    layout->addStretch();

//    connect(m_buttons->get(Cancel), &QPushButton::clicked,
//            this, &SignupVerificationWidget::onCancelClicked);
//    connect(m_buttons->get(CompleteSignup), &QPushButton::clicked,
//            this, &SignupVerificationWidget::onCompleteSignupClicked);
//    connect(m_buttons->get(ResendSignupCode), &QPushButton::clicked,
//            this, &SignupVerificationWidget::onResendSignupCodeClicked);
//    connect(ApiManager::instance(), &ApiManager::completeSignupSuccessful,
//            this, &SignupVerificationWidget::onCompleteSignupSuccessful);
//    connect(ApiManager::instance(), &ApiManager::completeSignupFailure,
//            this, &SignupVerificationWidget::onCompleteSignupFailure);
//    connect(ApiManager::instance(), &ApiManager::resendSignupCodeSuccessful,
//            this, &SignupVerificationWidget::onResendSignupCodeSuccessful);
//    connect(ApiManager::instance(), &ApiManager::resendSignupCodeFailure,
//            this, &SignupVerificationWidget::onResendSignupCodeFailure);
//    connect(m_countdown, &Countdown::finished,
//            this, &SignupVerificationWidget::onCountdownFinished);
//    connect(ServerManager::instance(), &ServerManager::disconnected,
//            this, &SignupVerificationWidget::onServerDisconnected);
}
