#ifndef ORDERSUMMARYWIDGET_H
#define ORDERSUMMARYWIDGET_H

#include <QWidget>
#include <QDate>
#include <planinfo.h>

class QLabel;
class QGroupBox;
class QRadioButton;
class LineEdit;
class QPushButton;
class Switch;
class ButtonSlice;
class BusyIndicatorWidget;

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

private:
    PlanInfo m_planInfo;
    qint64 m_selectedPlan;
    QString m_cardNumber;
    QDate m_cardExpDate;
    QString m_cardCvv;
    QString m_fullName;
    QString m_email;
    QString m_phone;
    QString m_countryCode;
    QString m_state;
    QString m_city;
    QString m_address;
    QString m_postalCode;
    //
    Switch* m_termsSwitch;
    ButtonSlice* m_buttons;
    BusyIndicatorWidget* m_busyIndicator;
    //
    QGroupBox* m_orderSummaryGroup;
    QLabel* m_billingDetailsTitleLabel;
    QLabel* m_billingDetailsLabel;
    QLabel* m_paymentDetailsTitleLabel;
    QLabel* m_paymentDetailsLabel;
    QLabel* m_subscriptionDetailsTitleLabel;
    QLabel* m_subscriptionDetailsPlanLabel;
    QRadioButton* m_subscriptionDetailsMonthlyRadio;
    QRadioButton* m_subscriptionDetailsAnnuallyRadio;
    LineEdit* m_subscriptionDetailsCouponEdit;
    QPushButton* m_subscriptionDetailsCouponApplyButton;
    QLabel* m_priceDetailsTitleLabel;
    QLabel* m_priceDetailsSubscriptionLabel;
    QLabel* m_priceDetailsTaxesLabel;
    QLabel* m_priceDetailsTotalLabel;
    QLabel* m_priceDetailsPaymentCycleLabel;
};

#endif // ORDERSUMMARYWIDGET_H
