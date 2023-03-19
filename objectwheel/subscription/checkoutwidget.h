#ifndef CHECKOUTWIDGET_H
#define CHECKOUTWIDGET_H

#include <QWidget>
#include <QDate>
#include <planinfo.h>

class QLabel;
class QRadioButton;
class LineEdit;
class QPushButton;
class ButtonSlice;
class BusyIndicatorWidget;
class QAbstractButton;
class QSpacerItem;

class CheckoutWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CheckoutWidget)

public:
    explicit CheckoutWidget(QWidget* parent = nullptr);

public slots:
    void clear();
    void refresh(const PlanInfo& planInfo, qint64 selectedPlan,
                 const QString& cardNumber, const QDate& cardExpDate,
                 const QString& cardCvv, const QString& fullName,
                 const QString& email, const QString& phone, const QString& countryCode,
                 const QString& state, const QString& city, const QString& address,
                 const QString& postalCode);

private slots:
    void onSubscriptionTypeButtonToggled();
    void onApplyClearCouponButtonClicked();
    void onResponseCouponTest(int discountPercentage);
    void onPurchaseClicked();
    void onServerDisconnected();
    void onSubscriptionSuccessful();
    void onSubscriptionFailure();

signals:
    void done();
    void back(qreal price);

private:
    qreal m_discountPercentage;
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
    ButtonSlice* m_buttons;
    BusyIndicatorWidget* m_busyIndicator;
    //
    QLabel* m_billingDetailsTitleLabel;
    QLabel* m_billingDetailsLabel;
    QSpacerItem* m_billingDetailsSpacerItem;
    QLabel* m_paymentDetailsTitleLabel;
    QLabel* m_paymentDetailsLabel;
    QSpacerItem* m_paymentDetailsSpacerItem;
    QLabel* m_subscriptionDetailsTitleLabel;
    QLabel* m_subscriptionDetailsTypeLabel;
    QLabel* m_subscriptionDetailsPlanLabel;
    QRadioButton* m_subscriptionDetailsMonthlyRadio;
    QRadioButton* m_subscriptionDetailsAnnuallyRadio;
    LineEdit* m_subscriptionDetailsCouponEdit;
    QPushButton* m_subscriptionDetailsCouponApplyButton;
    QLabel* m_subscriptionDetailsFeeLabel;
    QLabel* m_subscriptionDetailsTaxesLabel;
    QLabel* m_subscriptionDetailsTotalLabel;
    QLabel* m_subscriptionDetailsPaymentCycleLabel;
};

#endif // CHECKOUTWIDGET_H
