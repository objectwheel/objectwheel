#include <subscriptionwidget.h>
#include <planwidget.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
#include <utilityfunctions.h>
#include <paintutils.h>

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
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/verification.svg"), QSize(60, 60), this));

    QFont f = UtilityFunctions::systemTitleFont();
    f.setWeight(QFont::Light);
    f.setPixelSize(24);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("Subscription Plans"));

    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    auto descriptionLabel = new QLabel(this);
    descriptionLabel->setFont(f);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setText(tr("Please select your subscription plan from below"));

    UtilityFunctions::adjustFontPixelSize(m_planWidget, -1);
    m_planWidget->setDefaultPlan(tr("Indie"));
    m_planWidget->setPlanBadge(tr("Indie"), tr("14-days\nFree\nTrial"));
    m_planWidget->setContentsMargins(0, 18, 0, 0);

    m_buttons->add(Purchase, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    m_buttons->get(Purchase)->setText(tr("Purchase"));
    m_buttons->get(Purchase)->setIcon(QIcon(QStringLiteral(":/images/welcome/ok.png")));
    m_buttons->get(Purchase)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = 100;
    m_buttons->triggerSettings();

    m_busyIndicator->setStyleSheet(QStringLiteral("background: transparent"));
    m_busyIndicator->setColor(palette().text().color());
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
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_planWidget, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

}
