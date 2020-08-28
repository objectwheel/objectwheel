#ifndef PAYMENTDETAILSWIDGET_H
#define PAYMENTDETAILSWIDGET_H

#include <QWidget>
#include <planinfo.h>

class BulkEdit;
class PaymentDetailsWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PaymentDetailsWidget)

public:
    explicit PaymentDetailsWidget(QWidget* parent = nullptr);

public slots:
    void refresh(const PlanInfo& planInfo, qint64 selectedPlan);

private slots:
    void cardNumberEditTextEditingFinished();
    void cardNumberEditTextEdited(QString text);
    void cardExpDateEditTextChanged(QString text);
    void onNextClicked();

signals:
    void back();
    void next(const PlanInfo& planInfo, qint64 selectedPlan,
              const QString& cardNumber, const QDate& cardExpDate,
              const QString& cardCvv, const QString& fullName,
              const QString& email, const QString& phone, const QString& countryCode,
              const QString& state, const QString& city, const QString& address,
              const QString& postalCode);

private:
    PlanInfo m_planInfo;
    qint64 m_selectedPlan;
    BulkEdit* m_bulkEdit;
};

#endif // PAYMENTDETAILSWIDGET_H
