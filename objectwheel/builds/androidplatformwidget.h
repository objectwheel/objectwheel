#ifndef ANDROIDPLATFORMWIDGET_H
#define ANDROIDPLATFORMWIDGET_H

#include <platformwidget.h>

class QLabel;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QPushButton;
class QListWidget;
class QCheckBox;
class QRadioButton;
class QToolButton;

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
    QCheckBox* m_includePemissionsCheck;
    QComboBox* m_permissionCombo;
    QListWidget* m_permissionList;
    QPushButton* m_addPermissionButton;
    QPushButton* m_removePermissionButton;
    QCheckBox* m_aabCheck;
    QCheckBox* m_abiArmeabiV7aCheck;
    QCheckBox* m_abiArm64V8aCheck;
    QCheckBox* m_abiX86Check;
    QCheckBox* m_abiX8664Check;
    QCheckBox* m_includeQtModulesCheck;
    QComboBox* m_qtModuleCombo;
    QListWidget* m_qtModuleList;
    QPushButton* m_addQtModuleButton;
    QPushButton* m_removeQtModuleButton;
    QRadioButton* m_signingDisabled;
    QRadioButton* m_signingEnabled;
    QLineEdit* m_keystorePathEdit;
    QPushButton* m_newKeystoreButton;
    QPushButton* m_browseKeystoreButton;
    QPushButton* m_clearKeystoreButton;
    QLineEdit* m_keystorePasswordEdit;
    QToolButton* m_showKeystorePasswordButton;
    QComboBox* m_keyAliasCombo;
    QLineEdit* m_keyPasswordEdit;
    QToolButton* m_showKeyPasswordButton;
    QCheckBox* m_sameAsKeystorePasswordCheck;
};

#endif // ANDROIDPLATFORMWIDGET_H
