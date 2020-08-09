#ifndef SUBSCRIPTIONWINDOW_H
#define SUBSCRIPTIONWINDOW_H

#include <QWidget>

class StackedLayout;
class SubscriptionWidget;
class CreditCardWidget;
class OrderSummaryWidget;
class SucceedWidget;
class ServerStatusWidget;

class SubscriptionWindow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SubscriptionWindow)

public:
    explicit SubscriptionWindow(QWidget* parent = nullptr);

    QSize sizeHint() const override;

public slots:
    void reset();

private:
    void resizeEvent(QResizeEvent* event) override;

signals:
    void done();

private:
    StackedLayout* m_stackedLayout;
    SubscriptionWidget* m_subscriptionWidget;
    CreditCardWidget* m_creditCardWidget;
    OrderSummaryWidget* m_orderSummaryWidget;
    SucceedWidget* m_succeedWidget;
    ServerStatusWidget* m_serverStatusWidget;
};

#endif // SUBSCRIPTIONWINDOW_H
