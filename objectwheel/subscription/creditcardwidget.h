#ifndef CREDITCARDWIDGET_H
#define CREDITCARDWIDGET_H

#include <QWidget>

class BulkEdit;
class CreditCardWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CreditCardWidget)

public:
    explicit CreditCardWidget(QWidget* parent = nullptr);

};

#endif // CREDITCARDWIDGET_H
