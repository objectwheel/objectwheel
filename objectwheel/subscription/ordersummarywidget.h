#ifndef ORDERSUMMARYWIDGET_H
#define ORDERSUMMARYWIDGET_H

#include <QWidget>
#include <planinfo.h>

class OrderSummaryWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OrderSummaryWidget)

public:
    explicit OrderSummaryWidget(QWidget* parent = nullptr);

public slots:
    void refresh(const PlanInfo& planInfo, qint64 selectedPlan,
                 const QString& cardNumber, const QDate& cardExpDate,
                 const QString& cardCvv, const QString& fullName,
                 const QString& email, const QString& phone, const QString& countryCode,
                 const QString& state, const QString& city, const QString& address,
                 const QString& postalCode);
};

#endif // ORDERSUMMARYWIDGET_H
