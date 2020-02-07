#ifndef PLATFORMCONTROLLER_H
#define PLATFORMCONTROLLER_H

#include <QCborMap>

class PlatformController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PlatformController)

public:
    explicit PlatformController(QObject* parent = nullptr);

    virtual QCborMap toCborMap() const = 0;

};

#endif // PLATFORMCONTROLLER_H
