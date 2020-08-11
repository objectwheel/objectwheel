#include <creditcardwidget.h>
#include <bulkedit.h>
#include <paintutils.h>
#include <buttonslice.h>

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>

enum Fields { CardNumber, CardCcv, CardDate};
enum Buttons { Next, Back };

CreditCardWidget::CreditCardWidget(QWidget* parent) : QWidget(parent)
  , m_selectedPlanLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/verification.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("Payment Details"));

    m_selectedPlanLabel->setAlignment(Qt::AlignHCenter);
    m_selectedPlanLabel->setStyleSheet(QStringLiteral("color: #77000000"));

    m_bulkEdit->add(CardNumber, tr("Card Number"));
    m_bulkEdit->add(CardDate, tr("Card Date"));
    m_bulkEdit->add(CardCcv, tr("Security Code"));
    m_bulkEdit->setFixedWidth(300);

    m_bulkEdit->get<QLineEdit*>(CardNumber)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CardDate)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CardCcv)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //    m_bulkEdit->get<QLineEdit*>(Code)->setValidator(
    //                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("^\\d{1,6}$")), this));

    auto buttons = new ButtonSlice(this);
    buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    buttons->add(Next, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    buttons->get(Back)->setText(tr("Back"));
    buttons->get(Next)->setText(tr("Next"));
    buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    buttons->get(Next)->setIcon(QIcon(QStringLiteral(":/images/welcome/load.png")));
    buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    buttons->settings().cellWidth = m_bulkEdit->sizeHint().width() / 2.0;
    buttons->triggerSettings();

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addSpacing(16);
    layout->addWidget(m_selectedPlanLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(buttons, 0, Qt::AlignHCenter);
    layout->addSpacing(16);
    layout->addStretch();

    connect(buttons->get(Back), &QPushButton::clicked,
            this, &CreditCardWidget::back);
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

void CreditCardWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan)
{
    int col = planInfo.columnForIdentifier(selectedPlan);
    qreal price = planInfo.price(col);
    qreal annualPrice = planInfo.annualPrice(col);
    QString priceText = tr("$%1/month").arg(price);
    if (annualPrice >= 0)
        priceText += tr(" ($%2 if paid annually)").arg(annualPrice);
    m_planInfo = planInfo;
    m_selectedPlan = selectedPlan;
    m_selectedPlanLabel->setText(tr("<span style=\"font-weight: 500\">Plan: </span>%1<br>"
                                    "<span style=\"font-weight: 500\">Price: </span>%2<br><br>"
                                    "Please enter your payment card<br>"
                                    "details below to continue purchasing")
                                 .arg(planInfo.at(0, col)).arg(priceText));
}
