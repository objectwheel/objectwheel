#include <androidplatformcontroller.h>
#include <androidplatformwidget.h>
#include <utilityfunctions.h>

#include <QFile>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QRadioButton>
#include <QToolButton>
#include <QPushButton>
#include <QCborArray>

AndroidPlatformController::AndroidPlatformController(AndroidPlatformWidget* androidPlatformWidget,
                                                     QObject* parent)
    : AbstractPlatformController(parent)
    , m_androidPlatformWidget(androidPlatformWidget)
{
}

QCborMap AndroidPlatformController::toCborMap() const
{
    QString name = m_androidPlatformWidget->nameEdit()->text();
    if (name.isEmpty())
        name = m_androidPlatformWidget->nameEdit()->placeholderText();

    QString versionName = m_androidPlatformWidget->versionNameEdit()->text();
    if (versionName.isEmpty())
        versionName = m_androidPlatformWidget->versionNameEdit()->placeholderText();

    QCborMap map;
    map.insert(QLatin1String("platform"), QLatin1String("android"));
    map.insert(QLatin1String("name"), name);
    map.insert(QLatin1String("versionCode"), m_androidPlatformWidget->versionCodeSpin()->value());
    map.insert(QLatin1String("versionName"), versionName);
    map.insert(QLatin1String("organization"), m_androidPlatformWidget->organizationEdit()->text());
    map.insert(QLatin1String("domain"), m_androidPlatformWidget->domainEdit()->text());
    map.insert(QLatin1String("package"), m_androidPlatformWidget->packageEdit()->text());

    if (m_androidPlatformWidget->aabCheck()->isChecked())
        map.insert(QLatin1String("aab"), true);

    map.insert(QLatin1String("screenOrientation"), AndroidPlatformWidget::orientationMap.value(
                   m_androidPlatformWidget->screenOrientationCombo()->currentText()));
    map.insert(QLatin1String("minApiLevel"), AndroidPlatformWidget::apiLevelMap.value(
                   m_androidPlatformWidget->minApiLevelCombo()->currentText()));
    map.insert(QLatin1String("targetApiLevel"), AndroidPlatformWidget::apiLevelMap.value(
                   m_androidPlatformWidget->targetApiLevelCombo()->currentText()));

    QCborArray abis;
    if (m_androidPlatformWidget->abiArmeabiV7aCheck()->isChecked())
        abis.append(QLatin1String("armeabi-v7a"));
    if (m_androidPlatformWidget->abiArm64V8aCheck()->isChecked())
        abis.append(QLatin1String("arm64-v8a"));
    if (m_androidPlatformWidget->abiX86Check()->isChecked())
        abis.append(QLatin1String("x86"));
    if (m_androidPlatformWidget->abiX8664Check()->isChecked())
        abis.append(QLatin1String("x86_64"));
    if (!abis.isEmpty())
        map.insert(QLatin1String("abis"), abis);

    QCborArray qtModules;
    for (int i = 0; i < m_androidPlatformWidget->qtModuleList()->count(); ++i)
        qtModules.append(m_androidPlatformWidget->qtModuleList()->item(i)->text());
    if (!qtModules.isEmpty())
        map.insert(QLatin1String("qtModules"), qtModules);

    QCborArray permissions;
    for (int i = 0; i < m_androidPlatformWidget->permissionList()->count(); ++i)
        permissions.append(m_androidPlatformWidget->permissionList()->item(i)->text());
    if (!permissions.isEmpty())
        map.insert(QLatin1String("permissions"), permissions);

    if (m_androidPlatformWidget->signingEnabled()->isChecked()) {
        QCborMap signature;
        signature.insert(QLatin1String("keystorePassword"), m_androidPlatformWidget->keystorePasswordEdit()->text());
        signature.insert(QLatin1String("keyPassword"), m_androidPlatformWidget->keyPasswordEdit()->text());
        signature.insert(QLatin1String("keyAlias"), m_androidPlatformWidget->keyAliasCombo()->currentText());
        if (!m_androidPlatformWidget->keystorePathEdit()->text().isEmpty()) {
            QFile file(m_androidPlatformWidget->keystorePathEdit()->text());
            if (!file.open(QFile::ReadOnly)) {
                UtilityFunctions::showMessage(m_androidPlatformWidget,
                                              tr("File read error"),
                                              tr("We are unable to read the keystore file, make sure the "
                                                 "path is correct and the file is readable."),
                                              QMessageBox::Critical);
                return QCborMap();
            }
            signature.insert(QLatin1String("keystore"), file.readAll());
        }
        map.insert(QLatin1String("signature"), signature);
    }

    if (!m_androidPlatformWidget->iconPathEdit()->text().isEmpty()) {
        QFile file(m_androidPlatformWidget->iconPathEdit()->text());
        if (!file.open(QFile::ReadOnly)) {
            UtilityFunctions::showMessage(m_androidPlatformWidget,
                                          tr("File read error"),
                                          tr("We are unable to read the icon file, make sure the "
                                             "path is correct and the file is readable."),
                                          QMessageBox::Critical);
            return QCborMap();
        }
        map.insert(QLatin1String("icon"), file.readAll());
    }

    return map;
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
    m_androidPlatformWidget->screenOrientationCombo()->setCurrentText(QLatin1String("Unspecified"));
    m_androidPlatformWidget->minApiLevelCombo()->setCurrentText(QLatin1String("API 21: Android 5.0"));
    m_androidPlatformWidget->targetApiLevelCombo()->setCurrentText(QLatin1String("API 23: Android 6.0"));
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
