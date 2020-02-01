#ifndef PLATFORMWIDGET_H
#define PLATFORMWIDGET_H

#include <QWidget>

class PlatformWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlatformWidget)

public:
    explicit PlatformWidget(QWidget* parent = nullptr);

    virtual QCborMap toCborMap() const = 0;
};

#endif // PLATFORMWIDGET_H
