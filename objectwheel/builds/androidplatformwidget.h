#ifndef ANDROIDPLATFORMWIDGET_H
#define ANDROIDPLATFORMWIDGET_H

#include <platformwidget.h>

class QLabel;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QPushButton;
class QListWidget;

class AndroidPlatformWidget final : public PlatformWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AndroidPlatformWidget)

public:
    explicit AndroidPlatformWidget(QWidget* parent = nullptr);

    virtual QCborMap toCborMap() const override;

private:
    QLineEdit* m_labelEdit;
    QSpinBox* m_versionCodeSpin;
    QLineEdit* m_versionNameEdit;
    QLineEdit* m_organizationEdit;
    QLineEdit* m_domainEdit;
    QLineEdit* m_packageEdit;
    QComboBox* m_screenOrientationCombo;
    QComboBox* m_minSdkVersionCombo;
    QComboBox* m_targetSdkVersionCombo;
    QLabel* m_iconPictureLabel;
    QPushButton* m_browseIconButton;
    QPushButton* m_clearIconButton;
    QComboBox* m_permissionCombo;
    QListWidget* m_permissionList;
    QPushButton* m_addPermissionButton;
    QPushButton* m_removePermissionButton;
};

#endif // ANDROIDPLATFORMWIDGET_H
