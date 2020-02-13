#include <androidplatformcontroller.h>
#include <androidplatformwidget.h>

#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QRadioButton>
#include <QToolButton>
#include <QPushButton>

AndroidPlatformController::AndroidPlatformController(AndroidPlatformWidget* androidPlatformWidget,
                                                     QObject* parent)
    : PlatformController(parent)
    , m_androidPlatformWidget(androidPlatformWidget)
{
}

QCborMap AndroidPlatformController::toCborMap() const
{
}

void AndroidPlatformController::charge() const
{
    m_androidPlatformWidget->permissionCombo()->clear();
    m_androidPlatformWidget->qtModuleCombo()->clear();

    m_androidPlatformWidget->permissionCombo()->addItems(AndroidPlatformWidget::androidPermissionList);
    m_androidPlatformWidget->qtModuleCombo()->addItems(AndroidPlatformWidget::qtModuleMap.keys());

    m_androidPlatformWidget->qtModuleList()->clear();
    m_androidPlatformWidget->qtModuleList()->addItem(QLatin1String("Qt Svg"));
    m_androidPlatformWidget->qtModuleCombo()->removeItem(m_androidPlatformWidget->qtModuleCombo()->findText(QLatin1String("Qt Svg")));

    m_androidPlatformWidget->versionCodeSpin()->setValue(1);
    m_androidPlatformWidget->screenOrientationCombo()->setCurrentText(QStringLiteral("Unspecified"));
    m_androidPlatformWidget->minApiLevelCombo()->setCurrentText(QStringLiteral("API 21: Android 5.0"));
    m_androidPlatformWidget->targetApiLevelCombo()->setCurrentText(QStringLiteral("API 23: Android 6.0"));
    m_androidPlatformWidget->aabCheck()->setChecked(false);
    m_androidPlatformWidget->abiArmeabiV7aCheck()->setChecked(true);
    m_androidPlatformWidget->abiArm64V8aCheck()->setChecked(false);
    m_androidPlatformWidget->abiX86Check()->setChecked(false);
    m_androidPlatformWidget->abiX8664Check()->setChecked(false);
    m_androidPlatformWidget->includeQtModulesCheck()->setChecked(true);

    m_androidPlatformWidget->signingDisabled()->setChecked(true);
    m_androidPlatformWidget->keystorePathEdit()->clear();
    m_androidPlatformWidget->keystorePasswordEdit()->clear();
    m_androidPlatformWidget->keyAliasCombo()->clear();
    m_androidPlatformWidget->keyPasswordEdit()->clear();
    m_androidPlatformWidget->keyPasswordEdit()->setEchoMode(QLineEdit::Password);
    m_androidPlatformWidget->keystorePasswordEdit()->setEchoMode(QLineEdit::Password);
    m_androidPlatformWidget->keyPasswordEdit()->setEnabled(false);
    m_androidPlatformWidget->showKeyPasswordButton()->setEnabled(false);
    m_androidPlatformWidget->showKeystorePasswordButton()->setChecked(false);
    m_androidPlatformWidget->showKeyPasswordButton()->setChecked(false);
    m_androidPlatformWidget->sameAsKeystorePasswordCheck()->setChecked(true);

    m_androidPlatformWidget->addPermissionButton()->setEnabled(false);
    m_androidPlatformWidget->permissionList()->setEnabled(false);
    m_androidPlatformWidget->permissionCombo()->setEnabled(false);
    m_androidPlatformWidget->removePermissionButton()->setEnabled(false);
    m_androidPlatformWidget->removeQtModuleButton()->setEnabled(false);
    m_androidPlatformWidget->sameAsKeystorePasswordCheck()->setEnabled(false);
    m_androidPlatformWidget->keyPasswordEdit()->setEnabled(false);
    m_androidPlatformWidget->showKeyPasswordButton()->setEnabled(false);
    m_androidPlatformWidget->keyAliasCombo()->setEnabled(false);
    m_androidPlatformWidget->showKeystorePasswordButton()->setEnabled(false);
    m_androidPlatformWidget->keystorePasswordEdit()->setEnabled(false);
    m_androidPlatformWidget->newKeystoreButton()->setEnabled(false);
    m_androidPlatformWidget->browseKeystoreButton()->setEnabled(false);
    m_androidPlatformWidget->clearKeystoreButton()->setEnabled(false);
}

void AndroidPlatformController::discharge() const
{
    // TODO
}
