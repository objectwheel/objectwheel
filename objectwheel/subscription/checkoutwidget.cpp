#include <checkoutwidget.h>
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

enum Buttons { Back, Purchase };

CheckoutWidget::CheckoutWidget(QWidget* parent) : QWidget(parent)
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
  , m_orderSummaryGroup(new QGroupBox(this))
  , m_billingDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_billingDetailsLabel(new QLabel(m_orderSummaryGroup))
  , m_paymentDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_paymentDetailsLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsTitleLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsTypeLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsPlanLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsMonthlyRadio(new QRadioButton(m_orderSummaryGroup))
  , m_subscriptionDetailsAnnuallyRadio(new QRadioButton(m_orderSummaryGroup))
  , m_subscriptionDetailsCouponEdit(new LineEdit(m_orderSummaryGroup))
  , m_subscriptionDetailsCouponApplyButton(new QPushButton(m_orderSummaryGroup))
  , m_subscriptionDetailsFeeLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsTaxesLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsTotalLabel(new QLabel(m_orderSummaryGroup))
  , m_subscriptionDetailsPaymentCycleLabel(new QLabel(m_orderSummaryGroup))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/subscription/checkout.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("Checkout"));

    m_buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(Purchase, QLatin1String("#8BBB56"), QLatin1String("#6EA045"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Purchase)->setText(tr("Purchase"));
    m_buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    m_buttons->get(Purchase)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Purchase)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = 150;
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

    UtilityFunctions::adjustFontPixelSize(m_orderSummaryGroup, -1);
    UtilityFunctions::adjustFontWeight(m_billingDetailsTitleLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_paymentDetailsTitleLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_subscriptionDetailsTitleLabel, QFont::Medium);

    m_subscriptionDetailsMonthlyRadio->setFocusPolicy(Qt::NoFocus);
    m_subscriptionDetailsAnnuallyRadio->setFocusPolicy(Qt::NoFocus);

    m_billingDetailsTitleLabel->setText(tr("Billing Details"));
    m_paymentDetailsTitleLabel->setText(tr("Payment Details"));
    m_subscriptionDetailsTitleLabel->setText(tr("Subscription Details"));
    m_subscriptionDetailsTypeLabel->setText(tr("Type"));
    m_subscriptionDetailsMonthlyRadio->setText(tr("Monthly"));
    m_subscriptionDetailsAnnuallyRadio->setText(tr("Annually"));
    m_subscriptionDetailsCouponApplyButton->setText(tr("Apply"));
    m_subscriptionDetailsCouponEdit->setPlaceholderText(tr("Coupon Code"));

    m_subscriptionDetailsCouponApplyButton->setSizePolicy(
                QSizePolicy::Maximum, m_subscriptionDetailsCouponApplyButton->sizePolicy().verticalPolicy());
    m_orderSummaryGroup->setFixedWidth(260);
    m_subscriptionDetailsMonthlyRadio->setChecked(true);

    m_billingDetailsLabel->setWordWrap(true);
    m_subscriptionDetailsPaymentCycleLabel->setWordWrap(true);
    m_subscriptionDetailsPaymentCycleLabel->setAlignment(Qt::AlignCenter);
    m_subscriptionDetailsPaymentCycleLabel->setStyleSheet(QStringLiteral("QLabel {"
                                                                         "  color: #a5000000;"
                                                                         "  border: 1px solid #65885500;"
                                                                         "  border-radius: 4px;"
                                                                         "  background: #45ffbb00;"
                                                                         "  padding: 3px;"
                                                                         "  padding-left: 4px;"
                                                                         "  padding-right: 4px;"
                                                                         "}"));

    auto totalPriceLabel = new QLabel(tr("Total:"));
    UtilityFunctions::adjustFontWeight(totalPriceLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_subscriptionDetailsTotalLabel, QFont::Medium);

    m_subscriptionDetailsMonthlyRadio->setStyleSheet("spacing: 1px; margin: 0px; padding: 0px");
    m_subscriptionDetailsAnnuallyRadio->setStyleSheet("spacing: 1px; margin: 0px; padding: 0px");

    m_subscriptionDetailsMonthlyRadio->setCursor(Qt::PointingHandCursor);
    m_subscriptionDetailsAnnuallyRadio->setCursor(Qt::PointingHandCursor);
    m_subscriptionDetailsCouponApplyButton->setCursor(Qt::PointingHandCursor);

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

    auto couponLayout = new QHBoxLayout;
    couponLayout->setSpacing(2);
    couponLayout->setContentsMargins(0, 0, 0, 0);
    couponLayout->addWidget(m_subscriptionDetailsCouponEdit);
    couponLayout->addWidget(m_subscriptionDetailsCouponApplyButton);

    auto priceLayout = new QGridLayout;
    priceLayout->setSpacing(1);
    priceLayout->setContentsMargins(0, 0, 0, 0);
    priceLayout->addWidget(m_subscriptionDetailsTypeLabel, 0, 0);
    priceLayout->addWidget(m_subscriptionDetailsMonthlyRadio, 0, 1, Qt::AlignRight);
    priceLayout->addWidget(m_subscriptionDetailsAnnuallyRadio, 0, 2, Qt::AlignRight);
    priceLayout->addWidget(new QLabel(tr("Plan:")), 1, 0);
    priceLayout->addWidget(m_subscriptionDetailsPlanLabel, 1, 2, Qt::AlignRight);
    priceLayout->addWidget(new QLabel(tr("Fee:")), 2, 0);
    priceLayout->addWidget(m_subscriptionDetailsFeeLabel, 2, 2, Qt::AlignRight);
    priceLayout->addWidget(new QLabel(tr("Taxes:")), 3, 0);
    priceLayout->addWidget(m_subscriptionDetailsTaxesLabel, 3, 2, Qt::AlignRight);
    priceLayout->addWidget(priceHLine, 4, 0, 1, 3);
    priceLayout->addWidget(totalPriceLabel, 5, 0);
    priceLayout->addWidget(m_subscriptionDetailsTotalLabel, 5, 2, Qt::AlignRight);
    priceLayout->setColumnStretch(1, 1);

    auto orderSummaryLayout = new QVBoxLayout(m_orderSummaryGroup);
    orderSummaryLayout->setSpacing(1);
    orderSummaryLayout->setContentsMargins(6, 6, 6, 6);
    orderSummaryLayout->addStretch();
    orderSummaryLayout->addWidget(m_billingDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_billingDetailsLabel);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_paymentDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_paymentDetailsLabel);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_subscriptionDetailsTitleLabel);
    orderSummaryLayout->addLayout(priceLayout);
    orderSummaryLayout->addSpacing(3);
    orderSummaryLayout->addLayout(couponLayout);
    orderSummaryLayout->addSpacing(3);
    orderSummaryLayout->addWidget(m_subscriptionDetailsPaymentCycleLabel);

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
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(buttonGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled),
            this, &CheckoutWidget::onSubscriptionTypeButtonToggled);

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
#include <QDebug>
void CheckoutWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan,
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

    int col = m_planInfo.columnForIdentifier(selectedPlan);
    qreal price = m_planInfo.price(col);
    const QString& country = UtilityFunctions::countryFromCode(m_countryCode);

    if (price > 0) {
        m_subscriptionDetailsTypeLabel->show();
        m_billingDetailsTitleLabel->show();
        m_billingDetailsLabel->show();
        m_paymentDetailsTitleLabel->show();
        m_paymentDetailsLabel->show();
        m_subscriptionDetailsMonthlyRadio->show();
        m_subscriptionDetailsAnnuallyRadio->show();
        m_subscriptionDetailsCouponApplyButton->show();
        m_subscriptionDetailsCouponEdit->show();
        m_subscriptionDetailsPaymentCycleLabel->show();
        m_billingDetailsLabel->setText(
                    QString(QString(m_fullName + QLatin1Char('\n') +
                                    m_email + QLatin1Char('\n') + m_phone)
                            .split(QLatin1Char('\n'), Qt::SkipEmptyParts).join('\n') + QString::fromUtf8("•") +
                            QString(m_address + QLatin1Char('\n') + m_postalCode)
                            .split(QLatin1Char('\n'), Qt::SkipEmptyParts).join(' ') + QString::fromUtf8("•") +
                            QString(m_city + QLatin1Char('\n') +
                                    m_state + QLatin1Char('\n') + country)
                            .split(QLatin1Char('\n'), Qt::SkipEmptyParts).join('/'))
                    .split(QString::fromUtf8("•"), Qt::SkipEmptyParts).join('\n'));
        m_paymentDetailsLabel->setText(tr("Card number: ") + m_cardNumber + QLatin1Char('\n') +
                                       tr("Expiration date: ") + m_cardExpDate.toString("MM'/'yy") + QLatin1String(", ") +
                                       tr("Cvv: ") + m_cardCvv);
        if (m_billingDetailsLabel->text().isEmpty()) {
            m_billingDetailsTitleLabel->hide();
            m_billingDetailsLabel->hide();
        }
    } else {
        m_subscriptionDetailsTypeLabel->hide();
        m_billingDetailsTitleLabel->hide();
        m_billingDetailsLabel->hide();
        m_paymentDetailsTitleLabel->hide();
        m_paymentDetailsLabel->hide();
        m_subscriptionDetailsMonthlyRadio->hide();
        m_subscriptionDetailsAnnuallyRadio->hide();
        m_subscriptionDetailsCouponApplyButton->hide();
        m_subscriptionDetailsCouponEdit->hide();
        m_subscriptionDetailsPaymentCycleLabel->hide();
    }

    bool isAnnual = m_subscriptionDetailsAnnuallyRadio->isChecked();
    qreal finalPrice = isAnnual ? m_planInfo.annualPrice(col) : m_planInfo.price(col);
    qreal finalTax = isAnnual ? m_planInfo.annualTax(col) : m_planInfo.tax(col);
    m_subscriptionDetailsPlanLabel->setText(m_planInfo.at(0, col));
    m_subscriptionDetailsFeeLabel->setText(QLatin1Char('$') + QString::number(finalPrice, 'f', 2));
    m_subscriptionDetailsTaxesLabel->setText(QLatin1Char('$') + QString::number(finalTax, 'f', 2));
    m_subscriptionDetailsTotalLabel->setText(QLatin1Char('$') + QString::number(finalPrice + finalTax, 'f', 2));

    if (price > 0) {
        if (isAnnual) {
            m_subscriptionDetailsPaymentCycleLabel->setText("Your card will be charged on the 25th "
                                                            "of each month starting from 25.09.2020");
        } else {
            m_subscriptionDetailsPaymentCycleLabel->setText(tr("Your card will be charged on the %1th "
                                                               "of each month starting from 25.09.2020"));
        }
    } else {

    }
}

void CheckoutWidget::onSubscriptionTypeButtonToggled()
{
    bool isAnnual = m_subscriptionDetailsAnnuallyRadio->isChecked();
    int col = m_planInfo.columnForIdentifier(m_selectedPlan);
    qreal price = m_planInfo.price(col);
    qreal finalPrice = isAnnual ? m_planInfo.annualPrice(col) : m_planInfo.price(col);
    qreal finalTax = isAnnual ? m_planInfo.annualTax(col) : m_planInfo.tax(col);
    m_subscriptionDetailsPlanLabel->setText(m_planInfo.at(0, col));
    m_subscriptionDetailsFeeLabel->setText(QLatin1Char('$') + QString::number(finalPrice, 'f', 2));
    m_subscriptionDetailsTaxesLabel->setText(QLatin1Char('$') + QString::number(finalTax, 'f', 2));
    m_subscriptionDetailsTotalLabel->setText(QLatin1Char('$') + QString::number(finalPrice + finalTax, 'f', 2));

    if (price > 0) {
        if (isAnnual) {
            m_subscriptionDetailsPaymentCycleLabel->setText(
                        tr("Your card will be charged on the <b>%1</b> "
                           "of each <b>year</b> starting from <b>%2</b>")
                        .arg(QDate::currentDate().addMonths(1).toString("d MMM"))
                        .arg(QDate::currentDate().addMonths(1).toString("d MMM yyyy")));
        } else {
            m_subscriptionDetailsPaymentCycleLabel->setText(
                        tr("Your card will be charged on the <b>%1th</b> "
                           "of each <b>month</b> starting from <b>%2</b>")
                        .arg(QDate::currentDate().addMonths(1).day())
                        .arg(QDate::currentDate().addMonths(1).toString("d MMM yyyy")));
        }
    } else {

    }
}
