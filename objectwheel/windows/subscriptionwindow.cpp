#include <subscriptionwindow.h>
#include <utilityfunctions.h>
#include <stackedlayout.h>
#include <serverstatuswidget.h>
#include <succeedwidget.h>
#include <appconstants.h>
#include <subscriptionwidget.h>
#include <paymentdetailswidget.h>
#include <checkoutwidget.h>
#include <usermanager.h>

SubscriptionWindow::SubscriptionWindow(QWidget* parent) : QWidget(parent)
  , m_stackedLayout(new StackedLayout(this))
  , m_subscriptionWidget(new SubscriptionWidget(this))
  , m_paymentDetailsWidget(new PaymentDetailsWidget(this))
  , m_checkoutWidget(new CheckoutWidget(this))
  , m_succeedWidget(new SucceedWidget(this))
  , m_serverStatusWidget(new ServerStatusWidget(this))
{
    resize(sizeHint()); // Don't use adjustSize() on Windows
    move(UtilityFunctions::centerPos(size()));
    // setAttribute(Qt::WA_QuitOnClose, false); Since its possible that it may be the last window
    setWindowTitle(AppConstants::LABEL);

    m_serverStatusWidget->adjustSize();
    m_serverStatusWidget->move(width() - m_serverStatusWidget->width() - 8, 8);
    m_serverStatusWidget->raise();
    connect(m_stackedLayout, &StackedLayout::currentChanged,
            m_serverStatusWidget, &ServerStatusWidget::raise);

    m_stackedLayout->setContentsMargins(0, 0, 0, 0);
    m_stackedLayout->addWidget(m_subscriptionWidget);
    m_stackedLayout->addWidget(m_paymentDetailsWidget);
    m_stackedLayout->addWidget(m_checkoutWidget);
    m_stackedLayout->addWidget(m_succeedWidget);

    connect(m_subscriptionWidget, &SubscriptionWidget::cancel,
            this, &SubscriptionWindow::done);
    connect(m_subscriptionWidget, &SubscriptionWidget::next,
            m_paymentDetailsWidget, &PaymentDetailsWidget::refresh);
    connect(m_subscriptionWidget, &SubscriptionWidget::next,
            this, [this] { m_stackedLayout->setCurrentWidget(m_paymentDetailsWidget); });
    connect(m_paymentDetailsWidget, &PaymentDetailsWidget::back,
            this, [this] { m_stackedLayout->setCurrentWidget(m_subscriptionWidget); });
    connect(m_paymentDetailsWidget, &PaymentDetailsWidget::next,
            m_checkoutWidget, &CheckoutWidget::refresh);
    connect(m_paymentDetailsWidget, &PaymentDetailsWidget::next,
            this, [this] { m_stackedLayout->setCurrentWidget(m_checkoutWidget); });
    connect(m_checkoutWidget, &CheckoutWidget::back,
            this, [this] { m_stackedLayout->setCurrentWidget(m_paymentDetailsWidget); });
//    connect(m_subscriptionWidget, &SubscriptionWidget::done, this, [=] (PlanManager::Plans plan)
//    {
//        UserManager::updatePlan(plan);
//        m_stackedLayout->setCurrentWidget(m_paymentDetailsWidget);
//        m_succeedWidget->play(tr("Thank you for purchasing"),
//                              tr("Your purchase is completed. Thank you for choosing us.\n"
//                                 "You can cancel your subscription anytime from the application preferences section."));
//    });
    /**** SucceedWidget settings ****/
    connect(m_succeedWidget, &SucceedWidget::done, this, [=]
    {
        m_stackedLayout->setCurrentWidget(m_subscriptionWidget);
        emit done();
    });
}

QSize SubscriptionWindow::sizeHint() const
{
    return QSize(980, 560);
}

void SubscriptionWindow::reset()
{
    m_stackedLayout->setCurrentWidget(m_subscriptionWidget);
    m_subscriptionWidget->refresh();
}

void SubscriptionWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_serverStatusWidget->move(width() - m_serverStatusWidget->width() - 8, 8);
}
