#ifndef GENERICPLATFORMWIDGET_H
#define GENERICPLATFORMWIDGET_H

#include <platformwidget.h>

class QLabel;
class QLineEdit;
class QSpinBox;

class GenericPlatformWidget final : public PlatformWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(GenericPlatformWidget)

public:
    explicit GenericPlatformWidget(QWidget* parent = nullptr);

    virtual QCborMap toCborMap() const override;

private:
    QLineEdit* m_labelEdit;
    QLineEdit* m_organizationEdit;
    QLineEdit* m_domainEdit;
    QSpinBox* m_versionCodeSpin;
};

#endif // GENERICPLATFORMWIDGET_H
