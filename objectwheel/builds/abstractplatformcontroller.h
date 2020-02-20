#ifndef ABSTRACTPLATFORMCONTROLLER_H
#define ABSTRACTPLATFORMCONTROLLER_H

#include <QCborMap>

class AbstractPlatformController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractPlatformController)

public:
    explicit AbstractPlatformController(QObject* parent = nullptr);

    virtual bool isComplete() const = 0;

    virtual QCborMap toCborMap() const = 0;

public slots:
    virtual void charge() const = 0;
    virtual void discharge() const = 0;
};

#endif // ABSTRACTPLATFORMCONTROLLER_H
