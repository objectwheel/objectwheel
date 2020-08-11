#ifndef SUBSCRIPTIONWIDGET_H
#define SUBSCRIPTIONWIDGET_H

#include <planwidget.h>

class BusyIndicatorWidget;
class SubscriptionWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SubscriptionWidget)

public:
    explicit SubscriptionWidget(QWidget* parent = nullptr);

public slots:
    void refresh();

private slots:
    void onNextButtonClicked();
    void onResponseSubscriptionPlans(const QByteArray& planData);
    void onServerDisconnected();

signals:
    void cancel();
    void next(const PlanInfo& planInfo, qint64 selectedPlan);

private:
    PlanWidget* m_planWidget;
    BusyIndicatorWidget* m_busyIndicator;
    bool m_isWaitingForConnection;
};

#endif // SUBSCRIPTIONWIDGET_H
