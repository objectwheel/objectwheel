#include <checkoutwidget.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
#include <lineedit.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <servermanager.h>
#include <apimanager.h>
#include <usermanager.h>

#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QBoxLayout>
#include <QButtonGroup>

enum Buttons { Back, Purchase };

CheckoutWidget::CheckoutWidget(QWidget* parent) : QWidget(parent)
  , m_discountPercentage(0)
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
  , m_billingDetailsTitleLabel(new QLabel(this))
  , m_billingDetailsLabel(new QLabel(this))
  , m_paymentDetailsTitleLabel(new QLabel(this))
  , m_paymentDetailsLabel(new QLabel(this))
  , m_subscriptionDetailsTitleLabel(new QLabel(this))
  , m_subscriptionDetailsTypeLabel(new QLabel(this))
  , m_subscriptionDetailsPlanLabel(new QLabel(this))
  , m_subscriptionDetailsMonthlyRadio(new QRadioButton(this))
  , m_subscriptionDetailsAnnuallyRadio(new QRadioButton(this))
  , m_subscriptionDetailsCouponEdit(new LineEdit(this))
  , m_subscriptionDetailsCouponApplyButton(new QPushButton(this))
  , m_subscriptionDetailsFeeLabel(new QLabel(this))
  , m_subscriptionDetailsTaxesLabel(new QLabel(this))
  , m_subscriptionDetailsTotalLabel(new QLabel(this))
  , m_subscriptionDetailsPaymentCycleLabel(new QLabel(this))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/subscription/checkout.svg"),
                                            QSize(60, 60), this));

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

    ////////

    auto orderSummaryGroup = new QGroupBox(tr("Order Summary"), this);
    orderSummaryGroup->setFixedWidth(260);
    orderSummaryGroup->setAlignment(Qt::AlignHCenter);

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
                QSizePolicy::Maximum,
                m_subscriptionDetailsCouponApplyButton->sizePolicy().verticalPolicy());
    m_subscriptionDetailsMonthlyRadio->setChecked(true);
    m_billingDetailsLabel->setWordWrap(true);
    m_subscriptionDetailsPaymentCycleLabel->setWordWrap(true);
    m_subscriptionDetailsPaymentCycleLabel->setAlignment(Qt::AlignCenter);

    auto totalPriceLabel = new QLabel(tr("Total:"), this);
    UtilityFunctions::adjustFontWeight(totalPriceLabel, QFont::Medium);
    UtilityFunctions::adjustFontWeight(m_subscriptionDetailsTotalLabel, QFont::Medium);

    m_subscriptionDetailsMonthlyRadio->setCursor(Qt::PointingHandCursor);
    m_subscriptionDetailsAnnuallyRadio->setCursor(Qt::PointingHandCursor);
    m_subscriptionDetailsCouponApplyButton->setCursor(Qt::PointingHandCursor);

    auto buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_subscriptionDetailsMonthlyRadio);
    buttonGroup->addButton(m_subscriptionDetailsAnnuallyRadio);

    QPalette fp(palette());
    fp.setColor(QPalette::WindowText, QColor("#c0c0c0"));
    auto priceHLine = new QFrame(this);
    priceHLine->resize(118, 1);
    priceHLine->setFrameShape(QFrame::HLine);
    priceHLine->setFrameShadow(QFrame::Plain);
    priceHLine->setPalette(fp);

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

    auto orderSummaryLayout = new QVBoxLayout(orderSummaryGroup);
    orderSummaryLayout->setSpacing(1);
    orderSummaryLayout->setContentsMargins(6, 6, 6, 6);
    orderSummaryLayout->addWidget(m_billingDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_billingDetailsLabel);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_paymentDetailsTitleLabel);
    orderSummaryLayout->addWidget(m_paymentDetailsLabel);
    orderSummaryLayout->addSpacing(8);
    orderSummaryLayout->addWidget(m_subscriptionDetailsTitleLabel);
    orderSummaryLayout->addLayout(priceLayout);
    orderSummaryLayout->addLayout(couponLayout);
    orderSummaryLayout->addSpacing(3);
    orderSummaryLayout->addWidget(m_subscriptionDetailsPaymentCycleLabel);
    orderSummaryLayout->addStretch();

    for (QWidget* child : orderSummaryGroup->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
        UtilityFunctions::adjustFontPixelSize(child, -1);

    m_subscriptionDetailsMonthlyRadio->setStyleSheet("spacing: 1px; margin: 0px; padding: 0px");
    m_subscriptionDetailsAnnuallyRadio->setStyleSheet("spacing: 1px; margin: 0px; padding: 0px");
    m_subscriptionDetailsPaymentCycleLabel->setStyleSheet(QStringLiteral("QLabel {"
                                                                         "  color: #a5000000;"
                                                                         "  border: 1px solid #65885500;"
                                                                         "  border-radius: 4px;"
                                                                         "  background: #45ffbb00;"
                                                                         "  padding: 3px;"
                                                                         "  padding-left: 4px;"
                                                                         "  padding-right: 4px;"
                                                                         "}"));

    ////////

    auto notesGroup = new QGroupBox(tr("Acknowledgements"), this);
    notesGroup->setAlignment(Qt::AlignHCenter);
    auto notesLabel = new QLabel(this);
    UtilityFunctions::adjustFontPixelSize(notesLabel, -1);
    notesLabel->setWordWrap(true);
    notesLabel->setOpenExternalLinks(true);
    notesLabel->setAlignment(Qt::AlignJustify);
    notesLabel->setText("<ul style='margin-left: 8px; -qt-list-indent: 0;'>"
                        "  <li style='margin-bottom: 8px;'>"
                        "    Applications that you are going to build with Objectwheel"
                        "    will make use of the Qt Toolkit, an opensource software"
                        "    library that is licensed under the GNU Lesser General Public"
                        "    License (LGPL) version 3, to work properly across multiple"
                        "    platforms."
                        "  </li>"
                        "  <li style='margin-bottom: 8px;'>"
                        "    The Qt Toolkit also makes use of some other opensource"
                        "    software libraries that are provided under various"
                        "    opensource licenses from the original authors."
                        "  </li>"
                        "  <li style='margin-bottom: 8px;'>"
                        "    Objectwheel acknowledges these opensource library usages."
                        "    You can use Objectwheel to build and distribute applications"
                        "    with licenses that are compatible with these opensource"
                        "    software licenses. We recommend that applications built with"
                        "    Objectwheel also acknowledge these usages, and quote these"
                        "    license statements in the About Section."
                        "  </li>"
                        "  <li style='margin-bottom: 8px;'>"
                        "    You can find a complete list of the libraries and their"
                        "    respective licenses that are used in the applications you"
                        "    build with Objectwheel here:"
                        "    <a href='https://doc-snapshots.qt.io/qt5-5.14/licenses-used-in-qt.html'>"
                        "    Licenses Used in Qt</a>"
                        "  </li>"
                        "</ul>");
    auto notesLayout = new QVBoxLayout(notesGroup);
    notesLayout->setSpacing(1);
    notesLayout->setContentsMargins(6, 6, 6, 6);
    notesLayout->addWidget(notesLabel);
    notesLayout->addStretch();

    ////////

    auto centralLayout = new QHBoxLayout;
    centralLayout->setSpacing(8);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addStretch(1);
    centralLayout->addWidget(notesGroup, 3);
    centralLayout->addWidget(orderSummaryGroup, 1);
    centralLayout->addStretch(1);

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

    //    connect(ServerManager::instance(), &ServerManager::disconnected,
    //            this, &SignupWidget::onServerDisconnected);
    //    connect(ApiManager::instance(), &ApiManager::signupSuccessful,
    //            this, &SignupWidget::onSignupSuccessful);
    //    connect(ApiManager::instance(), &ApiManager::signupFailure,
    //            this, &SignupWidget::onSignupFailure);
    connect(m_subscriptionDetailsCouponApplyButton, &QPushButton::clicked,
            this, &CheckoutWidget::onApplyCouponClearButtonClicked);
    connect(ApiManager::instance(), &ApiManager::responseCouponTest,
            this, &CheckoutWidget::onResponseCouponTest);
    connect(buttonGroup, qOverload<int, bool>(&QButtonGroup::buttonToggled),
            this, &CheckoutWidget::onSubscriptionTypeButtonToggled);
    connect(m_buttons->get(Purchase), &QPushButton::clicked,
            this, &CheckoutWidget::onPurchaseClicked);
    connect(m_buttons->get(Back), &QPushButton::clicked,
            this, &CheckoutWidget::back);
}

void CheckoutWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan,
                             const QString& cardNumber, const QDate& cardExpDate,
                             const QString& cardCvv, const QString& fullName,
                             const QString& email, const QString& phone,
                             const QString& countryCode, const QString& state,
                             const QString& city, const QString& address,
                             const QString& postalCode)
{
    m_discountPercentage = 0;
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

    using namespace UtilityFunctions;
    bool isAnnual = m_subscriptionDetailsAnnuallyRadio->isChecked();
    int col = m_planInfo.columnForIdentifier(selectedPlan);
    qint64 trialPeriod = m_planInfo.trialPeriod(col);
    qreal price = percentSmaller(m_planInfo.price(col), m_discountPercentage);
    qreal finalPrice = percentSmaller(isAnnual ? m_planInfo.annualPrice(col) : m_planInfo.price(col), m_discountPercentage);
    qreal finalTax = percentSmaller(isAnnual ? m_planInfo.annualTax(col) : m_planInfo.tax(col), m_discountPercentage);
    const QString& country = UtilityFunctions::countryFromCode(m_countryCode);

    m_subscriptionDetailsCouponEdit->clear();
    m_subscriptionDetailsCouponEdit->setEnabled(true);
    m_subscriptionDetailsCouponApplyButton->setText(tr("Apply"));

    if (trialPeriod < 0)
        trialPeriod = 0;

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
                                       tr("Card exp date: ") + m_cardExpDate.toString("MM'/'yy") + QLatin1String(", ") +
                                       tr("Card cvv: ") + m_cardCvv);
        if (isAnnual) {
            m_subscriptionDetailsPaymentCycleLabel->setText(
                        tr("Your card will be charged on the <b>%1</b> "
                           "of each <b>year</b> starting from <b>%2</b>")
                        .arg(QDate::currentDate().addDays(trialPeriod).toString("d MMM"))
                        .arg(QDate::currentDate().addDays(trialPeriod).toString("d MMM yyyy")));
        } else {
            m_subscriptionDetailsPaymentCycleLabel->setText(
                        tr("Your card will be charged on the <b>%1th</b> "
                           "of each <b>month</b> starting from <b>%2</b>")
                        .arg(QDate::currentDate().addDays(trialPeriod).day())
                        .arg(QDate::currentDate().addDays(trialPeriod).toString("d MMM yyyy")));
        }
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

    m_subscriptionDetailsPlanLabel->setText(m_planInfo.at(0, col));
    m_subscriptionDetailsFeeLabel->setText(QLatin1Char('$') + QString::number(finalPrice, 'f', 2));
    m_subscriptionDetailsTaxesLabel->setText(QLatin1Char('$') + QString::number(finalTax, 'f', 2));
    m_subscriptionDetailsTotalLabel->setText(QLatin1Char('$') + QString::number(finalPrice + finalTax, 'f', 2));
}

void CheckoutWidget::onSubscriptionTypeButtonToggled()
{
    using namespace UtilityFunctions;
    bool isAnnual = m_subscriptionDetailsAnnuallyRadio->isChecked();
    int col = m_planInfo.columnForIdentifier(m_selectedPlan);
    qint64 trialPeriod = m_planInfo.trialPeriod(col);
    qreal price = percentSmaller(m_planInfo.price(col), m_discountPercentage);
    qreal finalPrice = percentSmaller(isAnnual ? m_planInfo.annualPrice(col) : m_planInfo.price(col), m_discountPercentage);
    qreal finalTax = percentSmaller(isAnnual ? m_planInfo.annualTax(col) : m_planInfo.tax(col), m_discountPercentage);

    if (trialPeriod < 0)
        trialPeriod = 0;

    if (price > 0) {
        if (isAnnual) {
            m_subscriptionDetailsPaymentCycleLabel->setText(
                        tr("Your card will be charged on the <b>%1</b> "
                           "of each <b>year</b> starting from <b>%2</b>")
                        .arg(QDate::currentDate().addDays(trialPeriod).toString("d MMM"))
                        .arg(QDate::currentDate().addDays(trialPeriod).toString("d MMM yyyy")));
        } else {
            m_subscriptionDetailsPaymentCycleLabel->setText(
                        tr("Your card will be charged on the <b>%1th</b> "
                           "of each <b>month</b> starting from <b>%2</b>")
                        .arg(QDate::currentDate().addDays(trialPeriod).day())
                        .arg(QDate::currentDate().addDays(trialPeriod).toString("d MMM yyyy")));
        }
    }

    m_subscriptionDetailsPlanLabel->setText(m_planInfo.at(0, col));
    m_subscriptionDetailsFeeLabel->setText(QLatin1Char('$') + QString::number(finalPrice, 'f', 2));
    m_subscriptionDetailsTaxesLabel->setText(QLatin1Char('$') + QString::number(finalTax, 'f', 2));
    m_subscriptionDetailsTotalLabel->setText(QLatin1Char('$') + QString::number(finalPrice + finalTax, 'f', 2));
}

void CheckoutWidget::onApplyCouponClearButtonClicked()
{
    if (m_subscriptionDetailsCouponApplyButton->text() == tr("Apply")) {
        const QString& couponCode = m_subscriptionDetailsCouponEdit->text();
        if (couponCode.isEmpty() || couponCode.size() > 255) {
            UtilityFunctions::showMessage(
                        this,
                        tr("Invalid code"),
                        tr("Your coupon code is invalid."));
            return;
        }
        if (ServerManager::isConnected()) {
            ApiManager::requestCouponTest(UserManager::email(), UserManager::password(), couponCode);
            m_busyIndicator->start();
        } else {
            UtilityFunctions::showMessage(
                        this,
                        tr("Unable to connect to the server"),
                        tr("We are unable to connect to the server in order to verify your coupon "
                           "code. Please checkout your internet connection and try again later. "));
        }
    } else {
        m_discountPercentage = 0;
        m_subscriptionDetailsCouponEdit->clear();
        m_subscriptionDetailsCouponEdit->setEnabled(true);
        m_subscriptionDetailsCouponApplyButton->setText(tr("Apply"));
        onSubscriptionTypeButtonToggled();
    }
}

void CheckoutWidget::onResponseCouponTest(int discountPercentage)
{
    m_busyIndicator->stop();
    m_subscriptionDetailsCouponApplyButton->setText(tr("Clear ⌫"));
    m_subscriptionDetailsCouponEdit->setEnabled(false);
    m_discountPercentage = discountPercentage;
    onSubscriptionTypeButtonToggled();
    UtilityFunctions::showMessage(
                this,
                tr("Coupon code has been successfully applied"),
                tr("You have got <b>%%1</b> discount in your purchase, enjoy!").arg(m_discountPercentage),
                QMessageBox::Information);
}

void CheckoutWidget::onPurchaseClicked()
{

}

void CheckoutWidget::onServerDisconnected()
{
    if (m_busyIndicator->isSpinning()) {
        m_busyIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("<p>Connection lost to the server, please "
                                         "try again later.</p>"));
    }
}