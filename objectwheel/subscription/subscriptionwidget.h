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

public slots:
    void refresh();

private:
    PlanManager::Plans plan() const;

private slots:
    void onNextButtonClicked();
    void onResponseSubscriptionPlans(const QByteArray& planData);
    void onServerDisconnected();

signals:
    void cancel();
    void done();

private:
    PlanWidget* m_planWidget;
    BusyIndicatorWidget* m_busyIndicator;
    bool m_isDelayerWorking;
};

#endif // SUBSCRIPTIONWIDGET_H
