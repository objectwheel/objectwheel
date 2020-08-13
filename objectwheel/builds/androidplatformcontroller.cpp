#include <androidplatformcontroller.h>
#include <androidplatformwidget.h>
#include <utilityfunctions.h>
#include <lineedit.h>
#include <paintutils.h>

#include <QFile>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QRadioButton>
#include <QToolButton>
#include <QPushButton>
#include <QCborArray>
#include <QFileDialog>
#include <QImageReader>
#include <QLabel>
#include <QStandardPaths>

AndroidPlatformController::AndroidPlatformController(AndroidPlatformWidget* androidPlatformWidget,
                                                     QObject* parent)
    : AbstractPlatformController(parent)
    , m_androidPlatformWidget(androidPlatformWidget)
{
    connect(m_androidPlatformWidget->nameEdit(), &LineEdit::textEdited,
            this, &AndroidPlatformController::onNameEdit);
    connect(m_androidPlatformWidget->domainEdit(), &LineEdit::textEdited,
            this, &AndroidPlatformController::onDomainEdit);
    connect(m_androidPlatformWidget->packageEdit(), &LineEdit::textEdited,
            this, &AndroidPlatformController::onPackageEdit);
    connect(m_androidPlatformWidget->versionCodeSpin(), &QSpinBox::valueChanged,
            this, &AndroidPlatformController::onVersionSpinValueChange);
    connect(m_androidPlatformWidget->browseIconButton(), &QPushButton::clicked,
            this, &AndroidPlatformController::onBrowseIconButtonClick);
    connect(m_androidPlatformWidget->clearIconButton(), &QPushButton::clicked,
            this, &AndroidPlatformController::onClearIconButtonClick);
    connect(m_androidPlatformWidget->includePemissionsCheck(), &QCheckBox::clicked,
            this, &AndroidPlatformController::onIncludePemissionsCheckClick);
    connect(m_androidPlatformWidget->addPermissionButton(), &QPushButton::clicked,
            this, &AndroidPlatformController::onAddPermissionButtonClick);
    connect(m_androidPlatformWidget->removePermissionButton(), &QPushButton::clicked,
            this, &AndroidPlatformController::onRemovePermissionButtonClick);
    connect(m_androidPlatformWidget->permissionList(), &QListWidget::itemSelectionChanged,
            this, &AndroidPlatformController::onPermissionListItemSelectionChange);
    connect(m_androidPlatformWidget->includeQtModulesCheck(), &QCheckBox::clicked,
            this, &AndroidPlatformController::onIncludeQtModulesCheckClick);
    connect(m_androidPlatformWidget->addQtModuleButton(), &QPushButton::clicked,
            this, &AndroidPlatformController::onAddQtModuleButtonClick);
    connect(m_androidPlatformWidget->removeQtModuleButton(), &QPushButton::clicked,
            this, &AndroidPlatformController::onRemoveQtModuleButtonClick);
    connect(m_androidPlatformWidget->qtModuleList(), &QListWidget::itemSelectionChanged,
            this, &AndroidPlatformController::onQtModuleListItemSelectionChange);
    connect(m_androidPlatformWidget->abiArmeabiV7aCheck(), &QCheckBox::clicked,
            this, &AndroidPlatformController::onAbiCheckClick);
    connect(m_androidPlatformWidget->abiArm64V8aCheck(), &QCheckBox::clicked,
            this, &AndroidPlatformController::onAbiCheckClick);
    connect(m_androidPlatformWidget->abiX86Check(), &QCheckBox::clicked,
            this, &AndroidPlatformController::onAbiCheckClick);
    connect(m_androidPlatformWidget->abiX8664Check(), &QCheckBox::clicked,
            this, &AndroidPlatformController::onAbiCheckClick);
}

bool AndroidPlatformController::isComplete() const
{
    if (m_androidPlatformWidget->signingEnabled()->isChecked()
            && (m_androidPlatformWidget->keystorePathEdit()->text().isEmpty()
                || m_androidPlatformWidget->keystorePasswordEdit()->text().isEmpty()
                || m_androidPlatformWidget->keyPasswordEdit()->text().isEmpty()
                || m_androidPlatformWidget->keyAliasCombo()->currentText().isEmpty())) {
        UtilityFunctions::showMessage(m_androidPlatformWidget,
                                      tr("Signing issue"),
                                      tr("Signing enabled but not all the necessary information "
                                         "provided. Please either disable signing or fill in all "
                                         "the requeired fields."));
        return false;
    }
    return true;
}

QCborMap AndroidPlatformController::toCborMap() const
{
    QString name = m_androidPlatformWidget->nameEdit()->text();
    QString versionName = m_androidPlatformWidget->versionNameEdit()->text();
    QString organization = m_androidPlatformWidget->organizationEdit()->text();
    QString domain = m_androidPlatformWidget->domainEdit()->text();
    QString package = m_androidPlatformWidget->packageEdit()->text();

    if (name.isEmpty())
        name = m_androidPlatformWidget->nameEdit()->placeholderText();
    if (versionName.isEmpty())
        versionName = m_androidPlatformWidget->versionNameEdit()->placeholderText();
    if (organization.isEmpty())
        organization = m_androidPlatformWidget->organizationEdit()->placeholderText();
    if (domain.isEmpty())
        domain = m_androidPlatformWidget->domainEdit()->placeholderText();
    if (package.isEmpty())
        package = m_androidPlatformWidget->packageEdit()->placeholderText();

    QCborMap map;
    map.insert(QLatin1String("platform"), QLatin1String("android"));
    map.insert(QLatin1String("name"), name);
    map.insert(QLatin1String("versionName"), versionName);
    map.insert(QLatin1String("versionCode"), m_androidPlatformWidget->versionCodeSpin()->value());
    map.insert(QLatin1String("organization"), organization);
    map.insert(QLatin1String("domain"), domain);
    map.insert(QLatin1String("package"), package);
    map.insert(QLatin1String("screenOrientation"), AndroidPlatformWidget::orientationMap.value(
                   m_androidPlatformWidget->screenOrientationCombo()->currentText()));
    map.insert(QLatin1String("minApiLevel"), AndroidPlatformWidget::apiLevelMap.value(
                   m_androidPlatformWidget->minApiLevelCombo()->currentText()));
    map.insert(QLatin1String("targetApiLevel"), AndroidPlatformWidget::apiLevelMap.value(
                   m_androidPlatformWidget->targetApiLevelCombo()->currentText()));

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

    if (m_androidPlatformWidget->includePemissionsCheck()->isChecked()) {
        QCborArray permissions;
        for (int i = 0; i < m_androidPlatformWidget->permissionList()->count(); ++i)
            permissions.append(m_androidPlatformWidget->permissionList()->item(i)->text());
        if (!permissions.isEmpty())
            map.insert(QLatin1String("permissions"), permissions);
    }

    if (m_androidPlatformWidget->aabCheck()->isChecked())
        map.insert(QLatin1String("aab"), true);

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

    if (m_androidPlatformWidget->includeQtModulesCheck()->isChecked()) {
        QCborArray qtModules;
        for (int i = 0; i < m_androidPlatformWidget->qtModuleList()->count(); ++i) {
            qtModules.append(AndroidPlatformWidget::qtModuleMap.value(
                                 m_androidPlatformWidget->qtModuleList()->item(i)->text()));
        }
        if (!qtModules.isEmpty())
            map.insert(QLatin1String("qtModules"), qtModules);
    }

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

    return map;
}

void AndroidPlatformController::charge() const
{
    m_androidPlatformWidget->nameEdit()->clear();
    m_androidPlatformWidget->versionCodeSpin()->setValue(1);
    m_androidPlatformWidget->versionNameEdit()->clear();
    m_androidPlatformWidget->versionNameEdit()->setPlaceholderText(tr("Version 1.0"));
    m_androidPlatformWidget->organizationEdit()->clear();
    m_androidPlatformWidget->domainEdit()->clear();
    m_androidPlatformWidget->packageEdit()->clear();
    m_androidPlatformWidget->packageEdit()->setPlaceholderText(tr("com.example.myapplication"));
    m_androidPlatformWidget->screenOrientationCombo()->setCurrentText(QLatin1String("Unspecified"));
    m_androidPlatformWidget->minApiLevelCombo()->setCurrentText(QLatin1String("API 21: Android 5.0"));
    m_androidPlatformWidget->targetApiLevelCombo()->setCurrentText(QLatin1String("API 23: Android 6.0"));
    m_androidPlatformWidget->iconPathEdit()->setText(QString());
    m_androidPlatformWidget->iconPictureLabel()->setPixmap(QPixmap());
    m_androidPlatformWidget->includePemissionsCheck()->setChecked(false);
    m_androidPlatformWidget->permissionCombo()->setEnabled(false);
    m_androidPlatformWidget->permissionList()->setEnabled(false);
    m_androidPlatformWidget->addPermissionButton()->setEnabled(false);
    m_androidPlatformWidget->removePermissionButton()->setEnabled(false);
    m_androidPlatformWidget->permissionCombo()->clear();
    m_androidPlatformWidget->permissionList()->clear();
    m_androidPlatformWidget->permissionCombo()->addItems(AndroidPlatformWidget::androidPermissionList);
    m_androidPlatformWidget->aabCheck()->setChecked(false);
    m_androidPlatformWidget->abiArmeabiV7aCheck()->setChecked(true);
    m_androidPlatformWidget->abiArm64V8aCheck()->setChecked(false);
    m_androidPlatformWidget->abiX86Check()->setChecked(false);
    m_androidPlatformWidget->abiX8664Check()->setChecked(false);
    m_androidPlatformWidget->includeQtModulesCheck()->setChecked(true);
    m_androidPlatformWidget->qtModuleCombo()->setEnabled(true);
    m_androidPlatformWidget->qtModuleList()->setEnabled(true);
    m_androidPlatformWidget->addQtModuleButton()->setEnabled(true);
    m_androidPlatformWidget->removeQtModuleButton()->setEnabled(false);
    m_androidPlatformWidget->qtModuleCombo()->clear();
    m_androidPlatformWidget->qtModuleList()->clear();
    m_androidPlatformWidget->qtModuleCombo()->addItems(AndroidPlatformWidget::qtModuleMap.keys());
    m_androidPlatformWidget->qtModuleList()->addItem(QLatin1String("Svg"));
    m_androidPlatformWidget->qtModuleCombo()->removeItem(m_androidPlatformWidget->qtModuleCombo()->findText(QLatin1String("Svg")));

    m_androidPlatformWidget->signingDisabled()->setChecked(true);
    m_androidPlatformWidget->keystorePathEdit()->clear();
    m_androidPlatformWidget->keystorePasswordEdit()->clear();
    m_androidPlatformWidget->keyAliasCombo()->clear();
    m_androidPlatformWidget->keyPasswordEdit()->clear();
    m_androidPlatformWidget->keyPasswordEdit()->setEchoMode(LineEdit::Password);
    m_androidPlatformWidget->keystorePasswordEdit()->setEchoMode(LineEdit::Password);
    m_androidPlatformWidget->keyPasswordEdit()->setEnabled(false);
    m_androidPlatformWidget->showKeyPasswordButton()->setEnabled(false);
    m_androidPlatformWidget->showKeystorePasswordButton()->setChecked(false);
    m_androidPlatformWidget->showKeyPasswordButton()->setChecked(false);
    m_androidPlatformWidget->sameAsKeystorePasswordCheck()->setChecked(true);
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

void AndroidPlatformController::onNameEdit(QString name) const
{
    QString domain = m_androidPlatformWidget->domainEdit()->text();
    const QString& oldName = m_androidPlatformWidget->nameEdit()->oldText();
    const QString& package = m_androidPlatformWidget->packageEdit()->text();
    const bool isRelationPreserved = package == generatePackageName(domain, oldName);
    if (name.isEmpty() || domain.isEmpty()) {
        const QString& oldDomain = m_androidPlatformWidget->domainEdit()->oldText();
        const QString& testName = name.isEmpty() ? oldName : name;
        const QString& testDomain = domain.isEmpty() ? oldDomain : domain;
        const bool isTestRelationPreserved = package == generatePackageName(testDomain, testName);
        if (isTestRelationPreserved || package.isEmpty()) {
            if (name.isEmpty())
                name = tr("myapplication");
            if (domain.isEmpty())
                domain = tr("example.com");
            m_androidPlatformWidget->packageEdit()->clear();
            m_androidPlatformWidget->packageEdit()->setPlaceholderText(generatePackageName(domain, name));
        }
    } else if (isRelationPreserved || package.isEmpty()) {
        m_androidPlatformWidget->packageEdit()->setText(generatePackageName(domain, name));
    }
}

void AndroidPlatformController::onDomainEdit(QString domain) const
{
    QString name = m_androidPlatformWidget->nameEdit()->text();
    const QString& oldDomain = m_androidPlatformWidget->domainEdit()->oldText();
    const QString& package = m_androidPlatformWidget->packageEdit()->text();
    const bool isRelationPreserved = package == generatePackageName(oldDomain, name);
    if (name.isEmpty() || domain.isEmpty()) {
        const QString& oldName = m_androidPlatformWidget->nameEdit()->oldText();
        const QString& testName = name.isEmpty() ? oldName : name;
        const QString& testDomain = domain.isEmpty() ? oldDomain : domain;
        const bool isTestRelationPreserved = package == generatePackageName(testDomain, testName);
        if (isTestRelationPreserved || package.isEmpty()) {
            if (name.isEmpty())
                name = tr("myapplication");
            if (domain.isEmpty())
                domain = tr("example.com");
            m_androidPlatformWidget->packageEdit()->clear();
            m_androidPlatformWidget->packageEdit()->setPlaceholderText(generatePackageName(domain, name));
        }
    } else if (isRelationPreserved || package.isEmpty()) {
        m_androidPlatformWidget->packageEdit()->setText(generatePackageName(domain, name));
    }
}

void AndroidPlatformController::onPackageEdit(const QString& package) const
{
    QString name = m_androidPlatformWidget->nameEdit()->text();
    QString domain = m_androidPlatformWidget->domainEdit()->text();
    if (package.isEmpty()) {
        if (name.isEmpty())
            name = tr("myapplication");
        if (domain.isEmpty())
            domain = tr("example.com");
        m_androidPlatformWidget->packageEdit()->setPlaceholderText(generatePackageName(domain, name));
    }
}

void AndroidPlatformController::onVersionSpinValueChange(int value) const
{
    m_androidPlatformWidget->versionNameEdit()->setPlaceholderText(tr("Version %1.0").arg(value));
}

void AndroidPlatformController::onBrowseIconButtonClick() const
{
    QLabel* iconLabel = m_androidPlatformWidget->iconPictureLabel();
    QLineEdit* iconEdit = m_androidPlatformWidget->iconPathEdit();

    const QString& formats = QImageReader::supportedImageFormats().join(QByteArrayLiteral(" *."));
    const QString& title = tr("Choose Application Icon");
    const QString& filter = tr("Image Files (*.%1)").arg(formats);
    const QString& desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QString& filePath = QFileDialog::getOpenFileName(iconLabel, title, desktopPath, filter);

    if (!filePath.isEmpty()) {
        const int f = 2 * iconLabel->frameWidth();
        const QSize& iconSize = iconLabel->size() - QSize(f, f);
        const QPixmap pixmap(filePath);
        if (pixmap.isNull()) {
            UtilityFunctions::showMessage(iconLabel,
                                          tr("Invalid image"),
                                          tr("The image you have chosen seems to be invalid."));
        } else if (pixmap.width() < 256 || pixmap.height() < 256  ||
                   pixmap.width() > 1024  || pixmap.height() > 1024) {
            UtilityFunctions::showMessage(iconLabel,
                                          tr("Invalid image size"),
                                          tr("Application icon size must fit between [256px - 1024px] "
                                             "for all the dimensions. Bigger or smaller sizes are not "
                                             "applicable. We recommend using a 256×256px PNG file for "
                                             "best result."));
        } else {
            const QPixmap& picture = PaintUtils::pixmap(filePath, iconSize, iconLabel);
            iconEdit->setText(filePath);
            iconLabel->setPixmap(picture);
        }
    }
}

void AndroidPlatformController::onClearIconButtonClick() const
{
    m_androidPlatformWidget->iconPathEdit()->setText(QString());
    m_androidPlatformWidget->iconPictureLabel()->setPixmap(QPixmap());
}

void AndroidPlatformController::onIncludePemissionsCheckClick(bool checked) const
{
    m_androidPlatformWidget->permissionCombo()->setEnabled(checked);
    m_androidPlatformWidget->permissionList()->setEnabled(checked);
    m_androidPlatformWidget->addPermissionButton()->setEnabled(
                checked && m_androidPlatformWidget->permissionCombo()->count() > 0);
    m_androidPlatformWidget->removePermissionButton()->setEnabled(
                checked && !m_androidPlatformWidget->permissionList()->selectedItems().isEmpty());
}

void AndroidPlatformController::onAddPermissionButtonClick() const
{
    QPushButton* addButton = m_androidPlatformWidget->addPermissionButton();
    QComboBox* combo = m_androidPlatformWidget->permissionCombo();
    QListWidget* list = m_androidPlatformWidget->permissionList();
    if (combo->count() > 0) {
        list->addItem(combo->currentText());
        list->item(list->count() - 1)->setSelected(true);
        combo->removeItem(combo->currentIndex());
    }
    addButton->setEnabled(combo->count() > 0);
}

void AndroidPlatformController::onRemovePermissionButtonClick() const
{
    QPushButton* addButton = m_androidPlatformWidget->addPermissionButton();
    QPushButton* removeButton = m_androidPlatformWidget->removePermissionButton();
    QComboBox* combo = m_androidPlatformWidget->permissionCombo();
    QListWidget* list = m_androidPlatformWidget->permissionList();
    if (!list->selectedItems().isEmpty()) {
        const QListWidgetItem* item = list->selectedItems().first();
        combo->addItem(item->text());
        combo->model()->sort(0);
        if (!addButton->isEnabled())
            addButton->setEnabled(true);
        delete list->takeItem(list->row(item));
    }
    removeButton->setEnabled(!list->selectedItems().isEmpty());
}

void AndroidPlatformController::onPermissionListItemSelectionChange() const
{
    m_androidPlatformWidget->removePermissionButton()->setEnabled(
                !m_androidPlatformWidget->permissionList()->selectedItems().isEmpty());
}

void AndroidPlatformController::onIncludeQtModulesCheckClick(bool checked) const
{
    m_androidPlatformWidget->qtModuleCombo()->setEnabled(checked);
    m_androidPlatformWidget->qtModuleList()->setEnabled(checked);
    m_androidPlatformWidget->addQtModuleButton()->setEnabled(
                checked && m_androidPlatformWidget->qtModuleCombo()->count() > 0);
    m_androidPlatformWidget->removeQtModuleButton()->setEnabled(
                checked && !m_androidPlatformWidget->qtModuleList()->selectedItems().isEmpty());
}

void AndroidPlatformController::onAddQtModuleButtonClick() const
{
    QPushButton* addButton = m_androidPlatformWidget->addQtModuleButton();
    QComboBox* combo = m_androidPlatformWidget->qtModuleCombo();
    QListWidget* list = m_androidPlatformWidget->qtModuleList();
    if (combo->count() > 0) {
        list->addItem(combo->currentText());
        list->item(list->count() - 1)->setSelected(true);
        combo->removeItem(combo->currentIndex());
    }
    addButton->setEnabled(combo->count() > 0);
}

void AndroidPlatformController::onRemoveQtModuleButtonClick() const
{
    QPushButton* addButton = m_androidPlatformWidget->addQtModuleButton();
    QPushButton* removeButton = m_androidPlatformWidget->removeQtModuleButton();
    QComboBox* combo = m_androidPlatformWidget->qtModuleCombo();
    QListWidget* list = m_androidPlatformWidget->qtModuleList();
    if (!list->selectedItems().isEmpty()) {
        const QListWidgetItem* item = list->selectedItems().first();
        combo->addItem(item->text());
        combo->model()->sort(0);
        if (!addButton->isEnabled())
            addButton->setEnabled(true);
        delete list->takeItem(list->row(item));
    }
    removeButton->setEnabled(!list->selectedItems().isEmpty());
}

void AndroidPlatformController::onQtModuleListItemSelectionChange() const
{
    m_androidPlatformWidget->removeQtModuleButton()->setEnabled(
                !m_androidPlatformWidget->qtModuleList()->selectedItems().isEmpty());
}

void AndroidPlatformController::onAbiCheckClick()
{
    if (!m_androidPlatformWidget->abiArmeabiV7aCheck()->isChecked()
            && !m_androidPlatformWidget->abiArm64V8aCheck()->isChecked()
            && !m_androidPlatformWidget->abiX86Check()->isChecked()
            && !m_androidPlatformWidget->abiX8664Check()->isChecked())
        m_androidPlatformWidget->abiArmeabiV7aCheck()->setChecked(true);
}

QString AndroidPlatformController::generatePackageName(const QString& rawDomain, const QString& rawAppName) const
{
    QStringList pieces(rawDomain.split(QLatin1Char('.')));
    pieces.replaceInStrings(QRegularExpression("\\s"), QString());
    pieces.removeAll(QString()); // Clean from null, empty and blank items
    std::reverse(pieces.begin(), pieces.end());
    QString appName(rawAppName);
    appName.remove(QLatin1Char('.'));
    appName.remove(QRegularExpression("\\s"));
    pieces.append(appName);
    appName = pieces.join('.').toLower().normalized(QString::NormalizationForm_KD);
    appName.remove(QRegularExpression("[^\\.a-z0-9]"));
    return appName;
}
