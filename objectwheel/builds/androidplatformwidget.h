#ifndef ANDROIDPLATFORMWIDGET_H
#define ANDROIDPLATFORMWIDGET_H

#include <QWidget>
#include <QMap>

class QLabel;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QPushButton;
class QListWidget;
class QCheckBox;
class QRadioButton;
class QToolButton;

class AndroidPlatformWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AndroidPlatformWidget)

public:
    static const QMap<QString, QString> apiLevelMap;
    static const QMap<QString, QString> orientationMap;
    static const QMap<QString, QString> qtModuleMap;
    static const QStringList androidPermissionList;

public:
    explicit AndroidPlatformWidget(QWidget* parent = nullptr);

    QLineEdit* nameEdit() const;
    QSpinBox* versionCodeSpin() const;
    QLineEdit* versionNameEdit() const;
    QLineEdit* organizationEdit() const;
    QLineEdit* domainEdit() const;
    QLineEdit* packageEdit() const;
    QComboBox* screenOrientationCombo() const;
    QComboBox* minApiLevelCombo() const;
    QComboBox* targetApiLevelCombo() const;
    QLabel* iconPictureLabel() const;
    QPushButton* browseIconButton() const;
    QPushButton* clearIconButton() const;
    QCheckBox* includePemissionsCheck() const;
    QComboBox* permissionCombo() const;
    QListWidget* permissionList() const;
    QPushButton* addPermissionButton() const;
    QPushButton* removePermissionButton() const;
    QCheckBox* aabCheck() const;
    QCheckBox* abiArmeabiV7aCheck() const;
    QCheckBox* abiArm64V8aCheck() const;
    QCheckBox* abiX86Check() const;
    QCheckBox* abiX8664Check() const;
    QCheckBox* includeQtModulesCheck() const;
    QComboBox* qtModuleCombo() const;
    QListWidget* qtModuleList() const;
    QPushButton* addQtModuleButton() const;
    QPushButton* removeQtModuleButton() const;
    QRadioButton* signingDisabled() const;
    QRadioButton* signingEnabled() const;
    QLineEdit* keystorePathEdit() const;
    QPushButton* newKeystoreButton() const;
    QPushButton* browseKeystoreButton() const;
    QPushButton* clearKeystoreButton() const;
    QLineEdit* keystorePasswordEdit() const;
    QToolButton* showKeystorePasswordButton() const;
    QComboBox* keyAliasCombo() const;
    QLineEdit* keyPasswordEdit() const;
    QToolButton* showKeyPasswordButton() const;
    QCheckBox* sameAsKeystorePasswordCheck() const;

private:
    QLineEdit* m_nameEdit;
    QSpinBox* m_versionCodeSpin;
    QLineEdit* m_versionNameEdit;
    QLineEdit* m_organizationEdit;
    QLineEdit* m_domainEdit;
    QLineEdit* m_packageEdit;
    QComboBox* m_screenOrientationCombo;
    QComboBox* m_minApiLevelCombo;
    QComboBox* m_targetApiLevelCombo;
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
