#ifndef SUBSCRIPTIONWIDGET_H
#define SUBSCRIPTIONWIDGET_H

#include <QWidget>

class PlanWidget;
class ButtonSlice;
class BusyIndicatorWidget;

class SubscriptionWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SubscriptionWidget)

public:
    explicit SubscriptionWidget(QWidget* parent = nullptr);

private:
    PlanWidget* m_planWidget;
    ButtonSlice* m_buttons;
    BusyIndicatorWidget* m_busyIndicator;
};

#endif // SUBSCRIPTIONWIDGET_H
