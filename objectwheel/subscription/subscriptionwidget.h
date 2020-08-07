#ifndef SUBSCRIPTIONWIDGET_H
#define SUBSCRIPTIONWIDGET_H

#include <QWidget>
#include <planmanager.h>

class PlanWidget;
class BusyIndicatorWidget;

class SubscriptionWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SubscriptionWidget)

public:
    explicit SubscriptionWidget(QWidget* parent = nullptr);

private:
    PlanManager::Plans plan() const;

private slots:
    void onPurchaseButtonClicked();
    void onSubscriptionSuccessful();
    void onSubscriptionFailure();
    void onServerDisconnected();

signals:
    void done(PlanManager::Plans plan);

private:
    PlanWidget* m_planWidget;
    BusyIndicatorWidget* m_busyIndicator;
};

#endif // SUBSCRIPTIONWIDGET_H
