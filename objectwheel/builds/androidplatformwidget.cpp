#include <androidplatformwidget.h>
#include <utilityfunctions.h>

#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QRadioButton>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

// TODO: Disable wheel events for all the widgets from spin boxes to comboboxes
// TODO: Add a "description" label under icon like "Enter final application details below right
//       before requesting a cloud build for your project"
// TODO: Make sure each line edit and everything matches safety regular exp
//       except "password" edits, find a solution for this
// TODO: Set tooltips
// TODO: Checkout old version of the codes and see what we are lacking
// TODO: Make sure we warn user before selecting a keystore "warning uploading it to our servers"

static QStringList g_androidSdkList {
    "API 21: Android 5.0",
    "API 22: Android 5.1",
    "API 23: Android 6.0",
    "API 24: Android 7.0",
    "API 25: Android 7.1",
    "API 26: Android 8.0",
    "API 27: Android 8.1",
    "API 28: Android 9.0",
    "API 29: Android 10",
};

static QStringList correctModuleList(QStringList list)
{
    QStringList removed;
    QMutableListIterator<QString> i(list);
    while (i.hasNext()) {
        const QString& module = i.next();
        if (module.startsWith(QLatin1String("Qt 3D"))) {
            removed.append(module);
            i.remove();
        }
    }
    list.append(removed);
    return list;
}

static QStringList androidPermissionList()
{
    static QStringList androidPermissions;
    if (androidPermissions.isEmpty()) {
        QTextStream stream(UtilityFunctions::resourceData(QLatin1String(":/other/AndroidPermissionList.txt")));
        QString line;
        while (stream.readLineInto(&line))
            androidPermissions.append(line);
    }
    return androidPermissions;
}

static QJsonObject androidModuleList()
{
    static QJsonObject androidModules;
    if (androidModules.isEmpty()) {
        const QJsonDocument& document = QJsonDocument::fromJson(
                    UtilityFunctions::resourceData(QLatin1String(":/other/QtModuleListAndroid.json")));
        androidModules = document.object();
    }
    return androidModules;
}

AndroidPlatformWidget::AndroidPlatformWidget(QWidget* parent) : PlatformWidget(parent)
  , m_nameEdit(new QLineEdit(this))
  , m_versionCodeSpin(new QSpinBox(this))
  , m_versionNameEdit(new QLineEdit(this))
  , m_organizationEdit(new QLineEdit(this))
  , m_domainEdit(new QLineEdit(this))
  , m_packageEdit(new QLineEdit(this))
  , m_screenOrientationCombo(new QComboBox(this))
  , m_minSdkVersionCombo(new QComboBox(this))
  , m_targetSdkVersionCombo(new QComboBox(this))
  , m_iconPictureLabel(new QLabel(this))
  , m_browseIconButton(new QPushButton(this))
  , m_clearIconButton(new QPushButton(this))
  , m_includePemissionsCheck(new QCheckBox(this))
  , m_permissionCombo(new QComboBox(this))
  , m_permissionList(new QListWidget(this))
  , m_addPermissionButton(new QPushButton(this))
  , m_removePermissionButton(new QPushButton(this))
  , m_aabCheck(new QCheckBox(this))
  , m_abiArmeabiV7aCheck(new QCheckBox(this))
  , m_abiArm64V8aCheck(new QCheckBox(this))
  , m_abiX86Check(new QCheckBox(this))
  , m_abiX8664Check(new QCheckBox(this))
  , m_includeQtModulesCheck(new QCheckBox(this))
  , m_qtModuleCombo(new QComboBox(this))
  , m_qtModuleList(new QListWidget(this))
  , m_addQtModuleButton(new QPushButton(this))
  , m_removeQtModuleButton(new QPushButton(this))
  , m_signingDisabled(new QRadioButton(this))
  , m_signingEnabled(new QRadioButton(this))
  , m_keystorePathEdit(new QLineEdit(this))
  , m_newKeystoreButton(new QPushButton(this))
  , m_browseKeystoreButton(new QPushButton(this))
  , m_clearKeystoreButton(new QPushButton(this))
  , m_keystorePasswordEdit(new QLineEdit(this))
  , m_showKeystorePasswordButton(new QToolButton(this))
  , m_keyAliasCombo(new QComboBox(this))
  , m_keyPasswordEdit(new QLineEdit(this))
  , m_showKeyPasswordButton(new QToolButton(this))
  , m_sameAsKeystorePasswordCheck(new QCheckBox(this))
{
    auto nameLabel = new QLabel(tr("Name:"), this);
    auto versionCodeLabel = new QLabel(tr("Version code:"), this);
    auto versionNameLabel = new QLabel(tr("Version name:"), this);
    auto organizationLabel = new QLabel(tr("Organization:"), this);
    auto domainLabel = new QLabel(tr("Domain:"), this);

    auto generalGroupBox = new QGroupBox(tr("General"), this);
    auto generalLayout = new QGridLayout(generalGroupBox);
    generalLayout->setSpacing(6);
    generalLayout->setContentsMargins(8, 8, 8, 8);
    generalLayout->addWidget(nameLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_nameEdit, 0, 1, 1, 2);
    generalLayout->addWidget(versionCodeLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_versionCodeSpin, 1, 1);
    generalLayout->addWidget(versionNameLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_versionNameEdit, 2, 1, 1, 2);
    generalLayout->addWidget(organizationLabel, 3, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_organizationEdit, 3, 1, 1, 2);
    generalLayout->addWidget(domainLabel, 4, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_domainEdit, 4, 1, 1, 2);
    generalLayout->setColumnStretch(1, 1);
    generalLayout->setColumnStretch(2, 1);

    auto packageNameLabel = new QLabel(tr("Package name:"), this);
    auto screenOrientationLabel = new QLabel(tr("Screen orientation:"), this);
    auto minSdkVersionLabel = new QLabel(tr("Minimum required SDK:"), this);
    auto targetSdkVersionLabel = new QLabel(tr("Target SDK:"), this);
    auto iconLabel = new QLabel(tr("Icon:"), this);
    auto permissionLabel = new QLabel(tr("Permissions:"), this);
    auto autoDetectPemissionsCheck = new QCheckBox(tr("Automatically detected"), this);

    auto iconLayout = new QGridLayout;
    iconLayout->setSpacing(4);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(m_iconPictureLabel, 0, 0, 2, 1);
    iconLayout->addWidget(m_browseIconButton, 0, 1);
    iconLayout->addWidget(m_clearIconButton, 1, 1);

    auto permDetectionLayout = new QVBoxLayout;
    permDetectionLayout->setSpacing(0);
    permDetectionLayout->setContentsMargins(iconLayout->contentsMargins());
    permDetectionLayout->addWidget(autoDetectPemissionsCheck);
    permDetectionLayout->addWidget(m_includePemissionsCheck);

    auto permissionLayout = new QGridLayout;
    permissionLayout->setSpacing(iconLayout->spacing());
    permissionLayout->setContentsMargins(iconLayout->contentsMargins());
    permissionLayout->addLayout(permDetectionLayout, 0, 0);
    permissionLayout->addWidget(m_permissionCombo, 1, 0);
    permissionLayout->addWidget(m_addPermissionButton, 1, 1);
    permissionLayout->addWidget(m_permissionList, 2, 0, 2, 1);
    permissionLayout->addWidget(m_removePermissionButton, 2, 1);

    auto androidSpesificGroupBox = new QGroupBox(tr("Android Spesific"), this);
    auto androidSpesificLayout = new QGridLayout(androidSpesificGroupBox);
    androidSpesificLayout->setSpacing(generalLayout->spacing());
    androidSpesificLayout->setContentsMargins(generalLayout->contentsMargins());
    androidSpesificLayout->addWidget(packageNameLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_packageEdit, 0, 1, 1, 2);
    androidSpesificLayout->addWidget(screenOrientationLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_screenOrientationCombo, 1, 1);
    androidSpesificLayout->addWidget(minSdkVersionLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_minSdkVersionCombo, 2, 1);
    androidSpesificLayout->addWidget(targetSdkVersionLabel, 3, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_targetSdkVersionCombo, 3, 1);
    androidSpesificLayout->addWidget(iconLabel, 4, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addLayout(iconLayout, 4, 1);
    androidSpesificLayout->addWidget(permissionLabel, 5, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addLayout(permissionLayout, 5, 1, 1, 2);
    androidSpesificLayout->setColumnStretch(1, 1);
    androidSpesificLayout->setColumnStretch(2, 1);

    auto aabLabel = new QLabel(tr("Android App Bundle:"), this);
    auto abisLabel = new QLabel(tr("Build ABIs:"), this);
    auto qtModulesLabel = new QLabel(tr("Qt modules:"), this);
    auto autoDetectQtModulesCheck = new QCheckBox(tr("Automatically detected based on Qml imports"), this);

    auto abisLayout = new QVBoxLayout;
    abisLayout->setSpacing(0);
    abisLayout->setContentsMargins(iconLayout->contentsMargins());
    abisLayout->addWidget(m_abiArmeabiV7aCheck);
    abisLayout->addWidget(m_abiArm64V8aCheck);
    abisLayout->addWidget(m_abiX86Check);
    abisLayout->addWidget(m_abiX8664Check);

    auto qtDetectionLayout = new QVBoxLayout;
    qtDetectionLayout->setSpacing(0);
    qtDetectionLayout->setContentsMargins(iconLayout->contentsMargins());
    qtDetectionLayout->addWidget(autoDetectQtModulesCheck);
    qtDetectionLayout->addWidget(m_includeQtModulesCheck);

    auto qtModuleLayout = new QGridLayout;
    qtModuleLayout->setSpacing(iconLayout->spacing());
    qtModuleLayout->setContentsMargins(iconLayout->contentsMargins());
    qtModuleLayout->addLayout(qtDetectionLayout, 0, 0);
    qtModuleLayout->addWidget(m_qtModuleCombo, 1, 0);
    qtModuleLayout->addWidget(m_addQtModuleButton, 1, 1);
    qtModuleLayout->addWidget(m_qtModuleList, 2, 0, 2, 1);
    qtModuleLayout->addWidget(m_removeQtModuleButton, 2, 1);

    auto buildingGroupBox = new QGroupBox(tr("Building"), this);
    auto buildingLayout = new QGridLayout(buildingGroupBox);
    buildingLayout->setSpacing(generalLayout->spacing());
    buildingLayout->setContentsMargins(generalLayout->contentsMargins());
    buildingLayout->addWidget(aabLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    buildingLayout->addWidget(m_aabCheck, 0, 1, 1, 2);
    buildingLayout->addWidget(abisLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    buildingLayout->addLayout(abisLayout, 1, 1, 1, 2);
    buildingLayout->addWidget(qtModulesLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    buildingLayout->addLayout(qtModuleLayout, 2, 1, 1, 2);
    buildingLayout->setColumnStretch(1, 1);
    buildingLayout->setColumnStretch(2, 1);

    auto statusLabel = new QLabel(tr("Status:"), this);
    auto keystorePathLabel = new QLabel(tr("Key store path:"), this);
    auto keystorePasswordLabel = new QLabel(tr("Key store password:"), this);
    auto keyAliasLabel = new QLabel(tr("Key alias:"), this);
    auto keyPasswordLabel = new QLabel(tr("Key password:"), this);

    auto statusLayout = new QHBoxLayout;
    statusLayout->setSpacing(iconLayout->spacing());
    statusLayout->setContentsMargins(iconLayout->contentsMargins());
    statusLayout->addWidget(m_signingDisabled);
    statusLayout->addWidget(m_signingEnabled);
    statusLayout->addStretch();

    auto keystorePathLayout = new QGridLayout;
    keystorePathLayout->setSpacing(iconLayout->spacing());
    keystorePathLayout->setContentsMargins(iconLayout->contentsMargins());
    keystorePathLayout->addWidget(m_keystorePathEdit, 0, 0, 1, 4);
    keystorePathLayout->addWidget(m_newKeystoreButton, 1, 0);
    keystorePathLayout->addWidget(m_browseKeystoreButton, 1, 1);
    keystorePathLayout->addWidget(m_clearKeystoreButton, 1, 2);
    keystorePathLayout->setColumnStretch(3, 1);

    auto keystorePasswordLayout = new QHBoxLayout;
    keystorePasswordLayout->setSpacing(iconLayout->spacing());
    keystorePasswordLayout->setContentsMargins(iconLayout->contentsMargins());
    keystorePasswordLayout->addWidget(m_keystorePasswordEdit);
    keystorePasswordLayout->addWidget(m_showKeystorePasswordButton);

    auto keyPasswordLayout = new QGridLayout;
    keyPasswordLayout->setSpacing(iconLayout->spacing());
    keyPasswordLayout->setContentsMargins(iconLayout->contentsMargins());
    keyPasswordLayout->addWidget(m_keyPasswordEdit, 0, 0);
    keyPasswordLayout->addWidget(m_showKeyPasswordButton, 0, 1);
    keyPasswordLayout->addWidget(m_sameAsKeystorePasswordCheck, 1, 0);

    auto signingGroupBox = new QGroupBox(tr("Signing"), this);
    auto signingLayout = new QGridLayout(signingGroupBox);
    signingLayout->setSpacing(generalLayout->spacing());
    signingLayout->setContentsMargins(generalLayout->contentsMargins());
    signingLayout->addWidget(statusLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(statusLayout, 0, 1, 1, 2);
    signingLayout->addWidget(keystorePathLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(keystorePathLayout, 1, 1, 1, 2);
    signingLayout->addWidget(keystorePasswordLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(keystorePasswordLayout, 2, 1, 1, 2);
    signingLayout->addWidget(keyAliasLabel, 3, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addWidget(m_keyAliasCombo, 3, 1, 1, 2);
    signingLayout->addWidget(keyPasswordLabel, 4, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(keyPasswordLayout, 4, 1, 1, 2);
    signingLayout->setColumnStretch(1, 1);
    signingLayout->setColumnStretch(2, 1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(generalGroupBox);
    layout->addWidget(androidSpesificGroupBox);
    layout->addWidget(buildingGroupBox);
    layout->addWidget(signingGroupBox);

    m_signingDisabled->setCursor(Qt::PointingHandCursor);
    m_signingEnabled->setCursor(Qt::PointingHandCursor);
    m_newKeystoreButton->setCursor(Qt::PointingHandCursor);
    m_browseKeystoreButton->setCursor(Qt::PointingHandCursor);
    m_clearKeystoreButton->setCursor(Qt::PointingHandCursor);
    m_keyAliasCombo->setCursor(Qt::PointingHandCursor);
    m_showKeystorePasswordButton->setCursor(Qt::PointingHandCursor);
    m_showKeyPasswordButton->setCursor(Qt::PointingHandCursor);
    m_sameAsKeystorePasswordCheck->setCursor(Qt::PointingHandCursor);

    autoDetectPemissionsCheck->setChecked(true);
    autoDetectQtModulesCheck->setChecked(true);
    autoDetectPemissionsCheck->setEnabled(false);
    autoDetectQtModulesCheck->setEnabled(false);
    m_keystorePathEdit->setEnabled(false);
    m_showKeystorePasswordButton->setCheckable(true);
    m_showKeyPasswordButton->setCheckable(true);

    m_browseIconButton->setText(tr("Browse"));
    m_clearIconButton->setText(tr("Clear"));
    m_addPermissionButton->setText(tr("Add"));
    m_removePermissionButton->setText(tr("Remove"));
    m_addQtModuleButton->setText(tr("Add"));
    m_removeQtModuleButton->setText(tr("Remove"));
    m_aabCheck->setText(tr("Build AAB package instead of APK"));
    m_abiArmeabiV7aCheck->setText(tr("armeabi-v7a"));
    m_abiArm64V8aCheck->setText(tr("arm64-v8a"));
    m_abiX86Check->setText(tr("x86"));
    m_abiX8664Check->setText(tr("x86_64"));
    m_includeQtModulesCheck->setText(tr("Include manual additions below too:"));
    m_includePemissionsCheck->setText(tr("Include manual additions below too:"));
    m_signingDisabled->setText(tr("Disabled"));
    m_signingEnabled->setText(tr("Enabled"));
    m_newKeystoreButton->setText(tr("New"));
    m_browseKeystoreButton->setText(tr("Browse"));
    m_clearKeystoreButton->setText(tr("Clear"));
    m_sameAsKeystorePasswordCheck->setText(tr("Same as key store password"));

    m_browseIconButton->setIcon(QIcon(":/images/builds/browse.svg"));
    m_clearIconButton->setIcon(QIcon(":/images/designer/clear.svg"));
    m_addPermissionButton->setIcon(QIcon(":/images/designer/plus.svg"));
    m_removePermissionButton->setIcon(QIcon(":/images/designer/minus.svg"));
    m_addQtModuleButton->setIcon(QIcon(":/images/designer/plus.svg"));
    m_removeQtModuleButton->setIcon(QIcon(":/images/designer/minus.svg"));
    m_newKeystoreButton->setIcon(QIcon(":/images/designer/new-file.svg"));
    m_browseKeystoreButton->setIcon(QIcon(":/images/builds/browse.svg"));
    m_clearKeystoreButton->setIcon(QIcon(":/images/designer/clear.svg"));
    m_showKeystorePasswordButton->setIcon(QIcon(":/images/builds/show.svg"));
    m_showKeyPasswordButton->setIcon(QIcon(":/images/builds/show.svg"));

    UtilityFunctions::adjustFontWeight(m_keyPasswordEdit, QFont::Black);
    UtilityFunctions::adjustFontWeight(m_keystorePasswordEdit, QFont::Black);

    int iconPictureSize = m_browseIconButton->sizeHint().height()
            + iconLayout->spacing()
            + m_clearIconButton->sizeHint().height();
    m_iconPictureLabel->setFixedSize(iconPictureSize, iconPictureSize);
    m_iconPictureLabel->setFrameShape(QFrame::StyledPanel);

    int labelColMinSz = minSdkVersionLabel->sizeHint().width(); // Longest label's width
    generalLayout->setColumnMinimumWidth(0, labelColMinSz);
    androidSpesificLayout->setColumnMinimumWidth(0, labelColMinSz);
    buildingLayout->setColumnMinimumWidth(0, labelColMinSz);
    signingLayout->setColumnMinimumWidth(0, labelColMinSz);

    m_permissionCombo->addItems(androidPermissionList());
    m_qtModuleCombo->addItems(correctModuleList(androidModuleList().keys()));
    m_minSdkVersionCombo->addItems(g_androidSdkList);
    m_targetSdkVersionCombo->addItems(g_androidSdkList);
    m_permissionCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_qtModuleCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_minSdkVersionCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_targetSdkVersionCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);


    m_includeQtModulesCheck->setChecked(true);
    m_qtModuleList->clear();
    m_qtModuleList->addItem(QLatin1String("Qt Svg"));
    m_signingDisabled->setChecked(true);
    m_keystorePathEdit->clear();
    m_keystorePasswordEdit->clear();
    m_keyAliasCombo->clear();
    m_keyPasswordEdit->clear();
    m_keyPasswordEdit->setEchoMode(QLineEdit::Password);
    m_keystorePasswordEdit->setEchoMode(QLineEdit::Password);
    m_keyPasswordEdit->setEnabled(false);
    m_showKeyPasswordButton->setEnabled(false);
    m_showKeystorePasswordButton->setChecked(false);
    m_showKeyPasswordButton->setChecked(false);
    m_sameAsKeystorePasswordCheck->setChecked(true);

//    auto btnResIcon = new QToolButton;
////    btnResIcon->setIcon(QIcon(":/images/refresh.png"));
//    btnResIcon->setIconSize(QSize(14, 14));
//    btnResIcon->setToolTip("Reset icon to default.");
//    connect(btnResIcon, &QToolButton::clicked, [&]{
//        _txtIconPath.setText("");
//        _picIcon.setPixmap(QPixmap(":/images/android-default.png"));
//    });
//    _appBox.setTitle("Application Settings");
//    _appBox.setFixedHeight(280);
//    _appBoxLay.addWidget(&_lblAppName, 0, 0);
//    _appBoxLay.addWidget(&_txtAppName, 0, 1, 1, 2);
//    _appBoxLay.addWidget(&_lblVersionName, 1, 0);
//    _appBoxLay.addWidget(&_txtVersionName, 1, 1, 1, 2);
//    _appBoxLay.addWidget(&_lblVersionCode, 2, 0);
//    _appBoxLay.addWidget(&_spnVersionCode, 2, 1, 1, 2);
//    _appBoxLay.addWidget(&_lblOrientation, 3, 0);
//    _appBoxLay.addWidget(&_cmbOrientation, 3, 1, 1, 2);
//    _appBoxLay.addWidget(&_lblIcon, 4, 0);
//    _appBoxLay.addWidget(&_txtIconPath, 4, 1);
//    _appBoxLay.addWidget(&_btnIcon, 4, 2);
//    _appBoxLay.addWidget(btnResIcon, 4, 2);
//    _appBoxLay.addWidget(&_picIcon, 5, 1);
//    _appBoxLay.setColumnMinimumWidth(2, 55);
//    _appBoxLay.setAlignment(btnResIcon, Qt::AlignRight);

//    connect(&_btnIcon, &QToolButton::clicked, [&] {
//        auto fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
//          QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
//          tr("Image Files (*.png *.jpg *.jpeg *.tiff *.bmp)"));

//        QPixmap icon(fileName);
//        if (icon.width() == icon.height() &&
//            icon.width() >= 256 && icon.height() >= 256) {
//            _picIcon.setPixmap(icon);
//            _txtIconPath.setText(fileName);
//        } else if (!fileName.isEmpty()){
//            UtilityFunctions::showMessage(
//                        this, tr("Invalid"),
//                        tr("Image file is not valid. It must be atleast "
//                           "256 x 256 and width must be equal to height."));
//        }
//    });

//    connect(&_txtIconPath, &QLineEdit::textChanged, [&] {
//        if (_txtIconPath.text().isEmpty()) {
//            _txtIconPath.setToolTip(
//            "Select icon for your application. It must be at least 256 x 256 and\n"
//            "width must be equal to height. Known image formats such as PNG and\n"
//            "JPG are allowed. Your icon may contain alpha channel.");
//        } else {
//            _txtIconPath.setToolTip(_txtIconPath.text() + "\n" +
//            "Select icon for your application. It must be at least 256 x 256 and\n"
//            "width must be equal to height. Known image formats such as PNG and\n"
//            "JPG are allowed. Your icon may contain alpha channel.");
//        }
//    });

//    _lblAppName.setText("Application name:");
//    _lblVersionCode.setText("Version code:");
//    _lblVersionName.setText("Version name:");
//    _lblOrientation.setText("Orientation:");
//    _lblIcon.setText("Icon:");
//    _btnIcon.setText("...");
//    _btnIcon.setToolTip(
//    "Select icon for your application. It must be at least 256 x 256 and\n"
//    "width must be equal to height. Known image formats such as PNG and\n"
//    "JPG are allowed. Your icon may contain alpha channel.");

//    _txtIconPath.setDisabled(true);
//    _txtIconPath.setPlaceholderText("[Android Default]");
//    _txtIconPath.setToolTip(_btnIcon.toolTip());
//    _txtIconPath.setFixedWidth(130);

//    _picIcon.setToolTip("Application icon.");
//    _picIcon.setFixedSize({64, 64});
//    _picIcon.setScaledContents(true);
//    _picIcon.setPixmap(QPixmap(":/images/android-default.png"));
//    _picIcon.setFrameShape(QFrame::StyledPanel);

//    _txtAppName.setText("My App");
//    _txtAppName.setFixedWidth(200);
//    _txtAppName.setToolTip("Your application name.");

//    _txtVersionName.setText("1.0");
//    _txtVersionName.setFixedWidth(120);
//    _txtVersionName.setToolTip(
//    "The version number shown to users. This value can be set as a raw string. \n"
//    "The string has no other purpose than to be displayed to users. \n"
//    "The 'Version code' field holds the significant version number used internally.");

//    _spnVersionCode.setValue(1);
//    _spnVersionCode.setMaximum(99999);
//    _spnVersionCode.setFixedWidth(70);
//    _spnVersionCode.setToolTip(
//    "An internal version number. This number is used only to determine \n"
//    "whether one version is more recent than another, with higher \n"
//    "numbers indicating more recent versions. This is not the version \n"
//    "number shown to users; that number is set by the 'Version name' field. \n"
//    "\n"
//    "The value must be set as an integer, such as '100'. You can define it \n"
//    "however you want, as long as each successive version has a higher number. \n"
//    "For example, it could be a build number. Or you could translate a version \n"
//    "number in 'x.y' format to an integer by encoding the 'x' and 'y' \n"
//    "separately in the lower and upper 16 bits. Or you could simply increase \n"
//    "the number by one each time a new version is released.");

//    _cmbOrientation.setFixedWidth(120);
//    _cmbOrientation.setIconSize({14, 14});
//    _cmbOrientation.addItem(QIcon(":/images/free.png"), "Free");
//    _cmbOrientation.addItem(QIcon(":/images/landscape.png"), "Landscape");
//    _cmbOrientation.addItem(QIcon(":/images/portrait.png"), "Portrait");
//    _cmbOrientation.setToolTip(
//    "Application orientation. Use Landscape or Portrait to lock your \n"
//    "application orientation; or use Free to leave it unspecified.");

//    _packageBox.setTitle("Package Settings");
//    _packageBox.setFixedHeight(150);
//    _packageBoxLay.addWidget(&_lblPackageName, 0, 0);
//    _packageBoxLay.addWidget(&_txtPackageName, 0, 1);
//    _packageBoxLay.addWidget(&_lblMinSdk, 1, 0);
//    _packageBoxLay.addWidget(&_cmbMinSdk, 1, 1);
//    _packageBoxLay.addWidget(&_lblTargetSdk, 2, 0);
//    _packageBoxLay.addWidget(&_cmbTargetSdk, 2, 1);

//    _lblPackageName.setText("Package name:");
//    _lblMinSdk.setText("Minimum required SDK:");
//    _lblTargetSdk.setText("Target SDK:");
//    _txtPackageName.setText("com.example.myapp");

//    auto validator = new QRegExpValidator(
//    QRegExp("^([A-Za-z]{1}[A-Za-z\\d_]*\\.)*[A-Za-z][A-Za-z\\d_]*$"));
//    _txtPackageName.setValidator(validator);
//    _txtPackageName.setFixedWidth(200);
//    _txtPackageName.setToolTip(
//    "The Android package name, also known as the Google Play ID, the unique \n"
//    "identifier of an application. Please choose a valid package name for \n"
//    "your application (for example, 'org.example.myapp'). \n"
//    "\n"
//    "Packages are usually defined using a hierarchical naming pattern, with \n"
//    "levels in the hierarchy seperated by periods (.) (pronounced 'dot').\n"
//    "\n"
//    "In general, a package name begins with the top level domain name of \n"
//    "the organization and then the organization's domain and then any \n"
//    "subdomains listed in reverse order. The organization can then choose \n"
//    "a spesific name for their package. Package names should be all \n"
//    "lowercase characters whenever possible.\n"
//    "\n"
//    "Completed conventions for disambiguating package names and rules for \n"
//    "naming packages when the Internet domain name cannot be directly \n"
//    "used as a package name are described in section 7.7 of \n"
//    "the Java Language Specification.");

//    _cmbMinSdk.setFixedWidth(200);
//    _cmbMinSdk.setToolTip("Sets the minimum required version on which this application can be run.");
//    _cmbMinSdk.setCurrentText("API 17: Android 4.2, 4.2.2");

//    _cmbTargetSdk.setFixedWidth(200);
//    _cmbTargetSdk.setToolTip(
//    "Sets the target SDK. Set this to the highest tested version. This \n"
//    "disables compatibility behavior of the system for your application.");
//    _cmbTargetSdk.setCurrentText("API 21: Android 5.0");

//    _permissionsBox.setTitle("Permission Settings");
//    _permissionsBox.setFixedHeight(200);
//    _permissionsBoxLay.addWidget(&_permissionList, 0, 0);
//    _permissionsBoxLay.addWidget(&_btnDelPermission, 0, 1);
//    _permissionsBoxLay.addWidget(&_cmbPermissions, 1, 0);
//    _permissionsBoxLay.addWidget(&_btnAddPermission, 1, 1);
//    _permissionsBoxLay.setAlignment(&_btnDelPermission, Qt::AlignBottom);

//    _chkSign.setToolTip("You don't have to sign debugging apps, for debugging purpose\n"
//                        "for instance if you wont release it on Google Play Store.");
////  FIXME  QString line;
////    QTextStream stream(rdfile(":/other/android-permissions.txt"));
////    while (stream.readLineInto(&line))
////        _cmbPermissions.addItem(line);
//    _cmbPermissions.setFixedWidth(400);
//    _cmbPermissions.setMaxVisibleItems(15);
//    _cmbPermissions.setEditable(true);

//    _permissionList.setFocusPolicy(Qt::NoFocus);
//    _permissionList.setFixedWidth(400);
//    _permissionList.addItem("android.permission.WRITE_EXTERNAL_STORAGE");
//    _permissionList.addItem("android.permission.READ_PHONE_STATE");
//    _btnDelPermission.setDisabled(true);
//    connect(&_permissionList, &QListWidget::itemSelectionChanged, [&] {
//        auto ct = _permissionList.currentItem()->text();
//        _btnDelPermission.setEnabled(_permissionList.currentItem() && ct !=
//                "android.permission.READ_PHONE_STATE" && ct !=
//                "android.permission.WRITE_EXTERNAL_STORAGE");
//    });

//    connect(&_cmbPermissions, &QComboBox::currentTextChanged, [&] {
//        bool found = false;
//        auto ct = _cmbPermissions.currentText();
//        for (int i = 0; i < _permissionList.count(); i++) {
//            if (ct == _permissionList.item(i)->text()) {
//                found = true;
//                break;
//            }
//        }
//        _btnAddPermission.setDisabled(found);
//    });

//    connect(&_btnAddPermission, &QPushButton::clicked, [&] {
//        _permissionList.addItem(_cmbPermissions.currentText());
//        emit _cmbPermissions.currentTextChanged(QString());
//    });

//    connect(&_btnDelPermission, &QPushButton::clicked, [&] {
//        delete _permissionList.takeItem(_permissionList.currentRow());
//        emit _cmbPermissions.currentTextChanged(QString());
//    });

//    QPalette p4(_btnDelPermission.palette());
//    p4.setColor(_btnDelPermission.foregroundRole(), "#991223");
//    _btnDelPermission.setPalette(p4);
//    _permissionList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    _btnDelPermission.setText("Delete");
////    _btnDelPermission.setIcon(QIcon(":/images/delete.png"));
//    _btnAddPermission.setText("Add");
//    _btnAddPermission.setIcon(QIcon(":/images/newfile.png"));
//    _btnAddPermission.setCursor(Qt::PointingHandCursor);
//    _btnDelPermission.setCursor(Qt::PointingHandCursor);
//    _btnDelPermission.setToolTip("Delete selected permission from the list.");
//    _btnAddPermission.setToolTip("Add permission to the list.");
//    _permissionList.setToolTip("Permissions list. Some of the permissions are not shown here.\n"
//                               "Those permissions are needed by Qt Modules and they will be\n"
//                               "included automatically.\n");

//    _cmbPermissions.setToolTip("Select permission to add.");
//    _cmbPermissions.setValidator(validator);

//    _signingBox.setTitle("Signing Settings");
//    _signingBox.setFixedHeight(255);
//    _signingBoxLay.addWidget(&_lblSign, 0, 0);
//    _signingBoxLay.addWidget(&_chkSign, 0, 1, 1, 2);
//    _signingBoxLay.addWidget(&_lblKsPath, 1, 0);
//    _signingBoxLay.addWidget(&_txtKsPath, 1, 1);
//    _signingBoxLay.addWidget(&_btnExistingKs, 1, 2);
//    _signingBoxLay.addWidget(&_btnNewKs, 2, 1, 1, 2);
//    _signingBoxLay.addWidget(&_lblKsPw, 3, 0);
//    _signingBoxLay.addWidget(&_txtKsPw, 3, 1, 1, 2);
//    _signingBoxLay.addWidget(&_lblKsAlias, 4, 0);
//    _signingBoxLay.addWidget(&_cmbKsAlias, 4, 1, 1, 2);
//    _signingBoxLay.addWidget(&_lblKeyPw, 5, 0);
//    _signingBoxLay.addWidget(&_txtAliasPw, 5, 1, 1, 2);

//    _btnExistingKs.setCursor(Qt::PointingHandCursor);
//    _btnNewKs.setCursor(Qt::PointingHandCursor);
//    _btnIcon.setCursor(Qt::PointingHandCursor);
//    btnResIcon->setCursor(Qt::PointingHandCursor);

//    _lblKsPath.setText("Key store path:");
//    _btnNewKs.setText("Create new...");
//    _btnNewKs.setIcon(QIcon(":/images/newfile.png"));
//    _btnExistingKs.setText("...");
//    _lblKsPw.setText("Key store password:");
//    _lblKsAlias.setText("Alias:");
//    _lblKeyPw.setText("Alias password:");
//    _txtKsPath.setPlaceholderText("Select key store file...");
//    _txtKsPath.setDisabled(true);
//    _btnNewKs.setFixedWidth(200);
//    _txtKsPath.setFixedWidth(160);
//    _cmbKsAlias.setFixedWidth(200);
//    _txtAliasPw.setFixedWidth(200);
//    _txtKsPw.setFixedWidth(200);
//    _txtAliasPw.setEchoMode(QLineEdit::Password);
//    _txtKsPw.setEchoMode(QLineEdit::Password);

//    _chkSign.setChecked(false);
//    _chkSign.setText(_chkSign.isChecked() ? "Yes" : "No");
//    _lblKsPath.setEnabled(_chkSign.isChecked());
//    _btnExistingKs.setEnabled(_chkSign.isChecked());
//    _btnNewKs.setEnabled(_chkSign.isChecked());
//    _lblKsPw.setEnabled(_chkSign.isChecked());
//    _txtKsPw.setEnabled(_chkSign.isChecked());
//    _lblKsAlias.setEnabled(_chkSign.isChecked());
//    _cmbKsAlias.setEnabled(_chkSign.isChecked());
//    _lblKeyPw.setEnabled(_chkSign.isChecked());
//    _txtAliasPw.setEnabled(_chkSign.isChecked());
//    connect(&_chkSign, &QCheckBox::stateChanged, [&]{
//        _chkSign.setText(_chkSign.isChecked() ? "Yes" : "No");
//        _lblKsPath.setEnabled(_chkSign.isChecked());
//        _btnExistingKs.setEnabled(_chkSign.isChecked());
//        _btnNewKs.setEnabled(_chkSign.isChecked());
//        _lblKsPw.setEnabled(_chkSign.isChecked());
//        _txtKsPw.setEnabled(_chkSign.isChecked());
//        _lblKsAlias.setEnabled(_chkSign.isChecked());
//        _cmbKsAlias.setEnabled(_chkSign.isChecked());
//        _lblKeyPw.setEnabled(_chkSign.isChecked());
//        _txtAliasPw.setEnabled(_chkSign.isChecked());
//    });

//    _lblSign.setText("Sign package:");
//    _txtKsPath.setToolTip("Key store path.");
//    _btnExistingKs.setToolTip("Select key store file...");
//    _btnNewKs.setToolTip("Generate new key store and signing certificate.");
//    _txtKsPw.setToolTip("Type key store password.");
//    _cmbKsAlias.setToolTip("Choose which alias you want to use. First you have to\n"
//                           "enter key store password above, then possible aliases\n"
//                           "will be shown here.");
//    _txtAliasPw.setToolTip("Type the related alias's password.");

//    connect(&_btnExistingKs, &QToolButton::clicked, [&] {
//        auto fileName = QFileDialog::getOpenFileName(this, tr("Select Keystore File"),
//          QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
//          tr("Key store files (*.keystore *.jks)"));
//        if (!fileName.isEmpty())
//            _txtKsPath.setText(fileName);
//    });
//    connect(&_txtKsPath, &QLineEdit::textChanged, [&] {
//        if (_txtKsPath.text().isEmpty()) {
//            _txtKsPath.setToolTip("Key store path.");
//        } else {
//            _txtKsPath.setToolTip("Key store path:\n" + _txtKsPath.text());
//        }
//    });

////    _btnBack.settings().topColor = "#38A3F6";
////    _btnBack.settings().bottomColor = _btnBack.settings().topColor.darker(120);
////    _btnBack.settings().borderRadius = 7.5;
////    _btnBack.settings().textColor = Qt::white;
////    _btnBack.setFixedSize(200,28);
////    _btnBack.setIconSize(QSize(14,14));
////    _btnBack.setIcon(QIcon(":/images/welcome/unload.png"));
////    _btnBack.setText("Back");
////  FIXME  connect(&_btnBack, &FlarButton::clicked, [&]{
////        emit backClicked();
////    });

////    _btnBuild.settings().topColor = "#84BF52";
////    _btnBuild.settings().bottomColor = _btnBuild.settings().topColor.darker(120);
////    _btnBuild.settings().borderRadius = 7.5;
////    _btnBuild.settings().textColor = Qt::white;
////    _btnBuild.setFixedSize(200,28);
////    _btnBuild.setIconSize(QSize(14,14));
////    _btnBuild.setIcon(QIcon(":/images/welcome/load.png"));
////    _btnBuild.setText("Build");
//    // FIXME   connect(&_btnBuild, &FlarButton::clicked, this, &AndroidPlatformWidget::handleBtnBuildClicked);
}

QCborMap AndroidPlatformWidget::toCborMap() const
{

}

//bool AndroidPlatformWidget::checkFields()
//{
//    QString errorMessage;

//    if (_txtAppName.text().isEmpty())
//        errorMessage += "* Application name cannot be empty.\n";

//    if (_txtVersionName.text().isEmpty())
//        errorMessage += "* Version name cannot be empty.\n";

//    if (_txtPackageName.text().isEmpty())
//        errorMessage += "* Package name cannot be empty.\n";
//    else if (!_txtPackageName.text().contains(QRegExp
//      ("^([A-Za-z]{1}[A-Za-z\\d_]*\\.)*[A-Za-z][A-Za-z\\d_]*$")))
//        errorMessage += "* Package name is not valid. Example usage: com.example.myapp\n";

//    if (_cmbMinSdk.currentText().split(':').
//     first().remove("API ").toInt() > _cmbTargetSdk.
//      currentText().split(':').first().remove("API ").toInt())
//        errorMessage += "* Target SDK api level cannot be lower than Minimum required SDK api level.\n";

//    if (_chkSign.isChecked()) {
//        if (_txtKsPath.text().isEmpty())
//             errorMessage += "* You have to choose a valid key store file when signing is on.\n";
//        else if (_txtKsPw.text().isEmpty())
//            errorMessage += "* Key store password cannot be empty.\n";
//        else if (_txtKsPw.text().size() < 6)
//            errorMessage += "* Key store password is too short.\n";
//        else if (_cmbKsAlias.currentText().isEmpty())
//            errorMessage += "* Alias name cannot be empty, your key store file is not valid.\n";
//        else if (_txtAliasPw.text().isEmpty())
//            errorMessage += "* Alias password cannot be empty.\n";
//        else if (_txtAliasPw.text().size() < 6)
//            errorMessage += "* Alias password is too short.\n";
//    }

//    if (!errorMessage.isEmpty())
//        UtilityFunctions::showMessage(this, tr("Warning"), errorMessage);

//    return errorMessage.isEmpty();
//}

//void AndroidPlatformWidget::setTarget(const QString& target)
//{
//    _lblTitle.setText("Target: " + target);
//}

//void AndroidPlatformWidget::handleBtnBuildClicked()
//{
//    if (checkFields()) {
//        Build::set(TAG_APPNAME, _txtAppName.text());
//        Build::set(TAG_VERSIONNAME, _txtVersionName.text());
//        Build::set(TAG_VERSIONCODE, _spnVersionCode.value());
//        if (_cmbOrientation.currentText() == "Free")
//            Build::set(TAG_ORIENTATION, "unspecified");
//        else {
//            Build::set(TAG_ORIENTATION,
//              _cmbOrientation.currentText().toLower());
//        }
//        Build::setIcon(_txtIconPath.text());

//        Build::set(TAG_PACKAGENAME, _txtPackageName.text());
//        Build::set(TAG_MINAPI, _cmbMinSdk.currentText().
//                   split(':').first().remove("API ").toInt());
//        Build::set(TAG_TARGETAPI, _cmbTargetSdk.currentText().
//                   split(':').first().remove("API ").toInt());

//        Build::clearPermissions();
//        for (int i = 0; i < _permissionList.count(); i++) {
//            auto permission = _permissionList.item(i)->text();
//            Build::addPermission(permission);
//        }

//        Build::set(TAG_SIGNON, _chkSign.isChecked());
//        if (_chkSign.isChecked()) {
//            Build::setKeystore(_txtKsPath.text());
//            Build::set(TAG_KSPW, _txtKsPw.text());
//            Build::set(TAG_ALIASNAME, _cmbKsAlias.currentText());
//            Build::set(TAG_ALIASPW, _txtAliasPw.text());
//        }

//        emit downloadBuild();
//    }
//}

//TODO: Add "reset form" and critisize side effects
//TODO: Generate certificate
//TODO: Test and check alias and keystore passwords
//TODO: Show aliases
//TODO: Critisize removing build.json when build is done,
// or after closing builds screen
