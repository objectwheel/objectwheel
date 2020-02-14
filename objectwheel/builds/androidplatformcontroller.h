#ifndef ANDROIDPLATFORMCONTROLLER_H
#define ANDROIDPLATFORMCONTROLLER_H

#include <abstractplatformcontroller.h>

class AndroidPlatformWidget;
class AndroidPlatformController final : public AbstractPlatformController
{
    Q_OBJECT
    Q_DISABLE_COPY(AndroidPlatformController)

public:
    explicit AndroidPlatformController(AndroidPlatformWidget* androidPlatformWidget,
                                       QObject* parent = nullptr);

    QCborMap toCborMap() const override;

public slots:
    void charge() const override;
    void discharge() const override;

private:
    AndroidPlatformWidget* m_androidPlatformWidget;
};

#endif // ANDROIDPLATFORMCONTROLLER_H
