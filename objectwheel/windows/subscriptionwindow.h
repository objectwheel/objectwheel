#ifndef SUBSCRIPTIONWINDOW_H
#define SUBSCRIPTIONWINDOW_H

#include <QWidget>

class SubscriptionWindow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SubscriptionWindow)

public:
    explicit SubscriptionWindow(QWidget* parent = nullptr);

};


#endif // SUBSCRIPTIONWINDOW_H
