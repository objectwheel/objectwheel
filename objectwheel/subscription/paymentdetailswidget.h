#ifndef PAYMENTDETAILSWIDGET_H
#define PAYMENTDETAILSWIDGET_H

#include <QWidget>
#include <planinfo.h>

class QLabel;
class BulkEdit;

class PaymentDetailsWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PaymentDetailsWidget)

public:
    explicit PaymentDetailsWidget(QWidget* parent = nullptr);

public slots:
    void refresh(const PlanInfo& planInfo, qint64 selectedPlan);

signals:
    void back();
    void next();

private:
    PlanInfo m_planInfo;
    qint64 m_selectedPlan;
    QLabel* m_selectedPlanLabel;
    BulkEdit* m_bulkEdit;
};

#endif // PAYMENTDETAILSWIDGET_H
