#include <subscriptionwidget.h>
#include <planwidget.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <appconstants.h>

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>

enum { Purchase };

SubscriptionWidget::SubscriptionWidget(QWidget* parent) : QWidget(parent)
  , m_planWidget(new PlanWidget(":/other/plans.csv", this))
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/subscription/subscription.svg"), QSize(60, 60), this));

    QFont f = UtilityFunctions::systemTitleFont();
    f.setWeight(QFont::Light);
    f.setPixelSize(24);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("%1 Subscription").arg(AppConstants::NAME));

    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto descriptionLabel = new QLabel(this);
    descriptionLabel->setFont(f);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setText(tr("Please select your subscription plan from below"));

    auto noticeButton = new QPushButton(this);
    noticeButton->setFixedSize(18, 18);
    noticeButton->setCursor(Qt::PointingHandCursor);
    noticeButton->setToolTip(tr("A letter from %1").arg(AppConstants::NAME));
    noticeButton->setIcon(QIcon(":/images/output/info.svg"));
    noticeButton->setFlat(true);

    auto noticeLabel = new QLabel(this);
    noticeLabel->setText(tr("Please read the letter from our company before purchasing"));
    noticeLabel->setContentsMargins({4, 4, 4, 4});
    noticeLabel->setFixedWidth(noticeLabel->sizeHint().width() + noticeButton->width() + 4 + 4 + 4);
    noticeLabel->setStyleSheet(QStringLiteral("QLabel {"
                                              "  color: #a5000000;"
                                              "  border: 1px solid #65885500;"
                                              "  border-radius: 4px;"
                                              "  background: #45ffbb00;"
                                              "}"));

    auto noticeLayout = new QHBoxLayout(noticeLabel);
    noticeLayout->setSpacing(0);
    noticeLayout->setContentsMargins(0, 0, 0, 0);
    noticeLayout->addStretch();
    noticeLayout->addWidget(noticeButton, Qt::AlignVCenter);

    UtilityFunctions::adjustFontPixelSize(m_planWidget, -1);
    m_planWidget->setDefaultPlan(tr("Indie"));
    m_planWidget->setPlanBadge(tr("Indie"), tr("30-days\nFree\nTrial"));
    m_planWidget->setContentsMargins(0, 18, 0, 0);

    m_buttons->add(Purchase, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    m_buttons->get(Purchase)->setText(tr("Purchase"));
    m_buttons->get(Purchase)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
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

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_planWidget, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(noticeLabel, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(noticeButton, &QPushButton::clicked, this, [=] {
        UtilityFunctions::showMessage(
                    this,
                    tr("A letter from %1").arg(AppConstants::NAME),
                    tr("<p>Dear fellow %1 user, first of all we thank you for downloading "
                       "and using %1. We hope that you will enjoy the experience.</p>"
                       "<p>Before buying an %1 subscription plan, please remember that "
                       "%1 is still in the beta process. This beta version does not "
                       "represent the final quality of the product. It might have bugs and some "
                       "features might not be available at the moment. But we promise that we are "
                       "gonna work hard and update %1 as quickly as possible. On the "
                       "other hand, as the developers of it, we trust in %1 and we "
                       "believe that its quality is a decent example in reflecting our unique "
                       "understanding of product design.</p>"
                       "<p>We are very new as a company. We are barely getting started and we have "
                       "very limited resources. In this regard, every purchase that you are gonna "
                       "make is very valuable/critical for us to be able to continue doing what we "
                       "do. It will help a lot in the way to improve our product quality.</p>"
                       "<p>Finally, please do not worry about the details of the subscription "
                       "plans too much for the moment. They are subject to change and in every "
                       "change we make, we will make sure that the user is the main beneficary. We "
                       "value every user feedback and we believe we will make %1 better "
                       "together. Please do not hasitate to leave a feedback at our "
                       "<a href='%2'>forum</a>. Even though we are a very small team of people, we "
                       "will try as hard as possible to stay in touch with our users.</p>"
                       "<p>Thank you for your support :)</p>"
                       "<p>—<i>Mike & Ömer</i><br><b>%3</b></p>"
                       ).arg(AppConstants::NAME).arg(AppConstants::FORUM_URL).arg(AppConstants::COMPANY_FULL),
                    QMessageBox::Information);
    });
}
