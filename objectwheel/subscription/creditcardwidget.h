#ifndef CREDITCARDWIDGET_H
#define CREDITCARDWIDGET_H

#include <QWidget>
#include <planmanager.h>

class BulkEdit;
class CreditCardWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CreditCardWidget)

public:
    explicit CreditCardWidget(QWidget* parent = nullptr);

signals:
    void done(PlanManager::Plans plan, const QString& creditCardNumber,
              const QString& creditCardCcv, const QDate& creditCardDate);

public slots:
    void setPlan(PlanManager::Plans plan);

private:
    PlanManager::Plans m_plan;
//    BulkEdit* m_bulkEdit;
};

#endif // CREDITCARDWIDGET_H
