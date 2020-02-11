#ifndef ANDROIDPLATFORMCONTROLLER_H
#define ANDROIDPLATFORMCONTROLLER_H

#include <platformcontroller.h>

class AndroidPlatformWidget;
class AndroidPlatformController final : public PlatformController
{
    Q_OBJECT
    Q_DISABLE_COPY(AndroidPlatformController)

public:
    explicit AndroidPlatformController(AndroidPlatformWidget* androidPlatformWidget,
                                       QObject* parent = nullptr);

    QCborMap toCborMap() const override;

public slots:
    void charge() const;
    void discharge() const;

private:
    AndroidPlatformWidget* m_androidPlatformWidget;
};

#endif // ANDROIDPLATFORMCONTROLLER_H
