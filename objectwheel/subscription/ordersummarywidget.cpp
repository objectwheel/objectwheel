#include <ordersummarywidget.h>
#include <switch.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
#include <lineedit.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QBoxLayout>
#include <QButtonGroup>

enum Buttons { Back, Purchase};

OrderSummaryWidget::OrderSummaryWidget(QWidget* parent) : QWidget(parent)
  , m_termsSwitch(new Switch(this))
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
  , m_orderSummaryGroup(new QGroupBox(this))
  , m_billingDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_billingDetailsLabel(new QLabel(m_orderSummaryGroup))
  , m_paymentDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_paymentDetailsLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsPlanLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsMonthlyRadio(new QRadioButton(m_orderSummaryGroup))
  , m_subscriptionDetailsAnnuallyRadio(new QRadioButton(m_orderSummaryGroup))
  , m_subscriptionDetailsCouponEdit(new LineEdit(m_orderSummaryGroup))
  , m_subscriptionDetailsCouponApplyButton(new QPushButton(m_orderSummaryGroup))
  , m_priceDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_priceDetailsSubscriptionLabel(new QLabel(m_orderSummaryGroup))
  , m_priceDetailsTaxesLabel(new QLabel(m_orderSummaryGroup))
  , m_priceDetailsTotalLabel(new QLabel(m_orderSummaryGroup))
  , m_priceDetailsPaymentCycleLabel(new QLabel(m_orderSummaryGroup))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/subscription/order-summary.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("Order Summary"));

    auto termsWidget = new QWidget(this);
    termsWidget->setFixedSize(400, 35);
    termsWidget->setObjectName(QStringLiteral("termsWidget"));
    termsWidget->setStyleSheet(QStringLiteral("#termsWidget {"
                                              "    border-radius: %1;"
                                              "    Background: #12000000;"
                                              "    border: 1px solid #18000000;"
                                              "}").arg(int(termsWidget->height() / 2.0)));

    auto termsLabel = new QLabel(termsWidget);
    termsLabel->setFocusPolicy(Qt::NoFocus);
    termsLabel->setTextFormat(Qt::RichText);
    termsLabel->setOpenExternalLinks(true);
    termsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    termsLabel->setText(tr("I understand the outcome"));

    auto termsLayout = new QHBoxLayout(termsWidget);
    termsLayout->setSpacing(4);
    termsLayout->setContentsMargins(2, 0, 0, 0);
    termsLayout->addWidget(m_termsSwitch, 0, Qt::AlignVCenter);
    termsLayout->addWidget(termsLabel, 0, Qt::AlignVCenter);
    termsLayout->addStretch();

    m_buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(Purchase, QLatin1String("#8BBB56"), QLatin1String("#6EA045"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Purchase)->setText(tr("Purchase"));
    m_buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    m_buttons->get(Purchase)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Purchase)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = termsWidget->width() / 2.0;
    m_buttons->triggerSettings();

    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setMinimumTrailOpacity(5);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setNumberOfLines(12);
    m_busyIndicator->setLineLength(5);
    m_busyIndicator->setInnerRadius(4);
    m_busyIndicator->setLineWidth(2);

    //

    UtilityFunctions::adjustFontWeight(m_billingDetailsTitleLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_paymentDetailsTitleLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_subscriptionDetailsTitleLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_priceDetailsTitleLabel, QFont::Medium);

    m_subscriptionDetailsMonthlyRadio->setFocusPolicy(Qt::NoFocus);
    m_subscriptionDetailsAnnuallyRadio->setFocusPolicy(Qt::NoFocus);

    m_billingDetailsTitleLabel->setText(tr("Billing Details"));
    m_paymentDetailsTitleLabel->setText(tr("Payment Details"));
    m_subscriptionDetailsTitleLabel->setText(tr("Subscription Details"));
    m_priceDetailsTitleLabel->setText(tr("Price Details"));
    m_subscriptionDetailsMonthlyRadio->setText(tr("Monthly"));
    m_subscriptionDetailsAnnuallyRadio->setText(tr("Annually"));
    m_subscriptionDetailsCouponApplyButton->setText(tr("Apply"));
    m_subscriptionDetailsCouponEdit->setPlaceholderText(tr("Coupon Code"));

    m_subscriptionDetailsCouponApplyButton->setSizePolicy(QSizePolicy::Maximum,
                                                          m_subscriptionDetailsCouponApplyButton->sizePolicy().verticalPolicy());
    m_orderSummaryGroup->setFixedWidth(250);

    auto buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_subscriptionDetailsMonthlyRadio);
    buttonGroup->addButton(m_subscriptionDetailsAnnuallyRadio);

    auto priceHLine = new QFrame;
    priceHLine->resize(118, 3);
    priceHLine->setFrameShape(QFrame::HLine);
    priceHLine->setFrameShadow(QFrame::Sunken);

    auto mainVLine = new QFrame;
    mainVLine->resize(118, 3);
    mainVLine->setFrameShape(QFrame::VLine);
    mainVLine->setFrameShadow(QFrame::Sunken);

    auto paymentCycleLayout = new QHBoxLayout;
    paymentCycleLayout->setSpacing(2);
    paymentCycleLayout->setContentsMargins(0, 0, 0, 0);
    paymentCycleLayout->addWidget(m_subscriptionDetailsMonthlyRadio);
    paymentCycleLayout->addWidget(m_subscriptionDetailsAnnuallyRadio);
    paymentCycleLayout->addStretch();

    auto couponLayout = new QHBoxLayout;
    couponLayout->setSpacing(2);
    couponLayout->setContentsMargins(0, 0, 0, 0);
    couponLayout->addWidget(m_subscriptionDetailsCouponEdit);
    couponLayout->addWidget(m_subscriptionDetailsCouponApplyButton);

    auto priceLayout = new QGridLayout;
    priceLayout->setSpacing(2);
    priceLayout->setContentsMargins(0, 0, 0, 0);
    priceLayout->addWidget(new QLabel(tr("Subscription:")), 0, 0);
    priceLayout->addWidget(m_priceDetailsSubscriptionLabel, 0, 1, Qt::AlignRight);
    priceLayout->addWidget(new QLabel(tr("Taxes:")), 1, 0);
    priceLayout->addWidget(m_priceDetailsTaxesLabel, 1, 1, Qt::AlignRight);
    priceLayout->addWidget(priceHLine, 2, 0, 1, 2);
    priceLayout->addWidget(new QLabel(tr("Total:")), 3, 0);
    priceLayout->addWidget(m_priceDetailsTotalLabel, 3, 1, Qt::AlignRight);

    auto orderSummaryLayout = new QVBoxLayout(m_orderSummaryGroup);
    orderSummaryLayout->setSpacing(0);
    orderSummaryLayout->setContentsMargins(6, 6, 6, 6);
    orderSummaryLayout->addStretch();
    orderSummaryLayout->addWidget(m_billingDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_billingDetailsLabel);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_paymentDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_paymentDetailsLabel);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_subscriptionDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_subscriptionDetailsPlanLabel);
    orderSummaryLayout->addLayout(paymentCycleLayout);
    orderSummaryLayout->addLayout(couponLayout);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_priceDetailsTitleLabel);
    orderSummaryLayout->addLayout(priceLayout);
    orderSummaryLayout->addWidget(m_priceDetailsPaymentCycleLabel);

    auto centralLayout = new QHBoxLayout;
    centralLayout->setSpacing(6);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addStretch();
    centralLayout->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    centralLayout->addWidget(mainVLine);
    centralLayout->addWidget(m_orderSummaryGroup);
    centralLayout->addStretch();

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addLayout(centralLayout);
    layout->addWidget(termsWidget, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

//    connect(ServerManager::instance(), &ServerManager::disconnected,
//            this, &SignupWidget::onServerDisconnected);
//    connect(ApiManager::instance(), &ApiManager::signupSuccessful,
//            this, &SignupWidget::onSignupSuccessful);
//    connect(ApiManager::instance(), &ApiManager::signupFailure,
//            this, &SignupWidget::onSignupFailure);
//    connect(m_buttons->get(Next), &QPushButton::clicked,
//            this, &SignupWidget::onNextClicked);
//    connect(m_bulkEdit, &BulkEdit::returnPressed,
//            this, &SignupWidget::onNextClicked);
//    connect(m_buttons->get(Back), &QPushButton::clicked,
//            this, &SignupWidget::back);
}

void OrderSummaryWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan,
                                 const QString& cardNumber, const QDate& cardExpDate,
                                 const QString& cardCvv, const QString& fullName,
                                 const QString& email, const QString& phone,
                                 const QString& countryCode, const QString& state,
                                 const QString& city, const QString& address,
                                 const QString& postalCode)
{
    m_planInfo = planInfo;
    m_selectedPlan = selectedPlan;
    m_cardNumber = cardNumber;
    m_cardExpDate = cardExpDate;
    m_cardCvv = cardCvv;
    m_fullName = fullName;
    m_email = email;
    m_phone = phone;
    m_countryCode = countryCode;
    m_state = state;
    m_city = city;
    m_address = address;
    m_postalCode = postalCode;

    int col = planInfo.columnForIdentifier(selectedPlan);
    qreal price = planInfo.price(col);
    if (price == 0) {
        m_billingDetailsLabel->hide();
        m_billingDetailsTitleLabel->hide();
        m_paymentDetailsLabel->hide();
        m_paymentDetailsTitleLabel->hide();
        m_subscriptionDetailsTitleLabel->hide();
        m_subscriptionDetailsPlanLabel->hide();
        m_subscriptionDetailsMonthlyRadio->hide();
        m_subscriptionDetailsAnnuallyRadio->hide();
        m_subscriptionDetailsCouponApplyButton->hide();
        m_subscriptionDetailsCouponEdit->hide();
    } else {
        m_billingDetailsLabel->show();
        m_billingDetailsTitleLabel->show();
        m_paymentDetailsLabel->show();
        m_paymentDetailsTitleLabel->show();
        m_subscriptionDetailsTitleLabel->show();
        m_subscriptionDetailsPlanLabel->show();
        m_subscriptionDetailsMonthlyRadio->show();
        m_subscriptionDetailsAnnuallyRadio->show();
        m_subscriptionDetailsCouponApplyButton->show();
        m_subscriptionDetailsCouponEdit->show();
        m_billingDetailsLabel->setText(m_fullName + QLatin1Char('\n') +
                                       m_email + QLatin1Char('\n') +
                                       m_phone + QLatin1Char('\n') +
                                       m_address + QLatin1Char(' ') +
                                       m_postalCode + QLatin1Char('\n') +
                                       m_city + QLatin1Char('/') +
                                       m_state + QLatin1Char('/') +
                                       UtilityFunctions::countryFromCode(m_countryCode));
        m_paymentDetailsLabel->setText(tr("Card number: ") + m_cardNumber + QLatin1Char('\n') +
                                       tr("Expiration date: ") + m_cardExpDate.toString("MM'/'yy") + QLatin1String(", ") +
                                       tr("Cvv: ") + m_cardCvv);
        m_subscriptionDetailsPlanLabel->setText(tr("%1 (%2)")
                                                .arg(m_planInfo.at(0, col))
                                                .arg(m_planInfo.badge(col).simplified()));
    }
}
