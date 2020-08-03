#ifndef SUBSCRIPTIONWINDOW_H
#define SUBSCRIPTIONWINDOW_H

#include <QWidget>

class SubscriptionWindow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SubscriptionWindow)

public:
    explicit SubscriptionWindow(QWidget* parent = nullptr);

public:
    QSize sizeHint() const override;

signals:
    void done();
};


#endif // SUBSCRIPTIONWINDOW_H
