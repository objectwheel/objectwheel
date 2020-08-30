#include <ordersummarywidget.h>

OrderSummaryWidget::OrderSummaryWidget(QWidget* parent) : QWidget(parent)
{

}

void OrderSummaryWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan,
                                 const QString& cardNumber, const QDate& cardExpDate,
                                 const QString& cardCvv, const QString& fullName,
                                 const QString& email, const QString& phone,
                                 const QString& countryCode, const QString& state,
                                 const QString& city, const QString& address,
                                 const QString& postalCode)
{
    m_planInfo = planInfo;
    m_selectedPlan = selectedPlan;
    m_cardNumber = cardNumber;
    m_cardExpDate = cardExpDate;
    m_cardCvv = cardCvv;
    m_fullName = fullName;
    m_email = email;
    m_phone = phone;
    m_countryCode = countryCode;
    m_state = state;
    m_city = city;
    m_address = address;
    m_postalCode = postalCode;
}
