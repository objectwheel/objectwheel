#ifndef ORDERSUMMARYWIDGET_H
#define ORDERSUMMARYWIDGET_H

#include <QWidget>

class OrderSummaryWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OrderSummaryWidget)

public:
    explicit OrderSummaryWidget(QWidget* parent = nullptr);

};

#endif // ORDERSUMMARYWIDGET_H
