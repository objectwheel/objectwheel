#ifndef ANDROIDPLATFORMWIDGET_H
#define ANDROIDPLATFORMWIDGET_H

#include <platformwidget.h>

class QLabel;
class QLineEdit;
class QSpinBox;

class AndroidPlatformWidget final : public PlatformWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AndroidPlatformWidget)

public:
    explicit AndroidPlatformWidget(QWidget* parent = nullptr);

    virtual QCborMap toCborMap() const override;

private:
    QLineEdit* m_labelEdit;
    QLineEdit* m_organizationEdit;
    QLineEdit* m_domainEdit;
    QSpinBox* m_versionCodeSpin;
};

#endif // ANDROIDPLATFORMWIDGET_H
