#include <androidwidget.h>
#include <projectmanager.h>
#include <build.h>
#include <utilityfunctions.h>

AndroidWidget::AndroidWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(this)
    , _appBoxLay(&_appBox)
    , _packageBoxLay(&_packageBox)
    , _permissionsBoxLay(&_permissionsBox)
    , _signingBoxLay(&_signingBox)
{
    setFocusPolicy(Qt::NoFocus);

//    auto buttonsLay = new QHBoxLayout;
////    buttonsLay->addWidget(&_btnBack);
////    buttonsLay->addWidget(&_btnBuild);

//    _layout.setContentsMargins(20,
//      20, 20, 20);
//    _layout.setSpacing(10);
//    _layout.addWidget(&_lblLogo);
//    _layout.addWidget(&_lblTitle);
//    _layout.addWidget(&_lblMsg);
//    _layout.addWidget(&_scrollArea);
//    _layout.addLayout(buttonsLay);
//    _layout.setAlignment(&_lblLogo, Qt::AlignHCenter);
//    _layout.setAlignment(&_lblTitle, Qt::AlignHCenter);
//    _layout.setAlignment(&_lblMsg, Qt::AlignHCenter);
//    _layout.setAlignment(&_scrollArea, Qt::AlignHCenter);
//    _layout.setAlignment(buttonsLay, Qt::AlignHCenter);

//    _lblLogo.setFixedSize(50, 50);
//    _lblLogo.setPixmap(QPixmap(":/images/android.png"));
//    _lblLogo.setScaledContents(true);

//    QFont f;
//    f.setPixelSize(28);
//    f.setWeight(QFont::ExtraLight);
//    _lblTitle.setFont(f);
//    _lblTitle.setText("Target: Android");

//    f.setPixelSize(17);
//    _lblMsg.setFont(f);
//    _lblMsg.setText("Settings");

//    auto scrollAreaWidgetContents = new QWidget();
//    scrollAreaWidgetContents->setLayout(&_scrollAreaLay);
//    scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
//    scrollAreaWidgetContents->setStyleSheet
//      ("#scrollAreaWidgetContents{background: transparent;}");

//    QPalette p3(_scrollArea.palette());
//    p3.setColor(_scrollArea.backgroundRole(), "#d4d4d4");
//    _scrollArea.setPalette(p3);
//    _scrollArea.setAutoFillBackground(true);
//    _scrollArea.setWidgetResizable(true);
//    _scrollArea.setWidget(scrollAreaWidgetContents);
//    _scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    _scrollArea.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
//    _scrollArea.setFixedWidth(600);
//    _scrollAreaLay.setSpacing(30);
//    _scrollAreaLay.setContentsMargins(20,
//      30, 20, 30);
//    _scrollAreaLay.addWidget(&_appBox);
//    _scrollAreaLay.addWidget(&_packageBox);
//    _scrollAreaLay.addWidget(&_permissionsBox);
//    _scrollAreaLay.addWidget(&_signingBox);

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
//    _cmbMinSdk.addItem("API 14: Android 4.0, 4.0.1, 4.0.2");
//    _cmbMinSdk.addItem("API 15: Android 4.0.3, 4.0.4");
//    _cmbMinSdk.addItem("API 16: Android 4.1, 4.1.1");
//    _cmbMinSdk.addItem("API 17: Android 4.2, 4.2.2");
//    _cmbMinSdk.addItem("API 18: Android 4.3");
//    _cmbMinSdk.addItem("API 19: Android 4.4");
//    _cmbMinSdk.addItem("API 20: Android 4.4W");
//    _cmbMinSdk.addItem("API 21: Android 5.0");
//    _cmbMinSdk.addItem("API 22: Android 5.1");
//    _cmbMinSdk.addItem("API 23: Android 6.0");
//    _cmbMinSdk.setCurrentText("API 17: Android 4.2, 4.2.2");

//    _cmbTargetSdk.setFixedWidth(200);
//    _cmbTargetSdk.setToolTip(
//    "Sets the target SDK. Set this to the highest tested version. This \n"
//    "disables compatibility behavior of the system for your application.");
//    _cmbTargetSdk.addItem("API 14: Android 4.0, 4.0.1, 4.0.2");
//    _cmbTargetSdk.addItem("API 15: Android 4.0.3, 4.0.4");
//    _cmbTargetSdk.addItem("API 16: Android 4.1, 4.1.1");
//    _cmbTargetSdk.addItem("API 17: Android 4.2, 4.2.2");
//    _cmbTargetSdk.addItem("API 18: Android 4.3");
//    _cmbTargetSdk.addItem("API 19: Android 4.4");
//    _cmbTargetSdk.addItem("API 20: Android 4.4W");
//    _cmbTargetSdk.addItem("API 21: Android 5.0");
//    _cmbTargetSdk.addItem("API 22: Android 5.1");
//    _cmbTargetSdk.addItem("API 23: Android 6.0");
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

//    _btnBack.settings().topColor = "#38A3F6";
//    _btnBack.settings().bottomColor = _btnBack.settings().topColor.darker(120);
//    _btnBack.settings().borderRadius = 7.5;
//    _btnBack.settings().textColor = Qt::white;
//    _btnBack.setFixedSize(200,28);
//    _btnBack.setIconSize(QSize(14,14));
//    _btnBack.setIcon(QIcon(":/images/welcome/unload.png"));
//    _btnBack.setText("Back");
//  FIXME  connect(&_btnBack, &FlarButton::clicked, [&]{
//        emit backClicked();
//    });

//    _btnBuild.settings().topColor = "#84BF52";
//    _btnBuild.settings().bottomColor = _btnBuild.settings().topColor.darker(120);
//    _btnBuild.settings().borderRadius = 7.5;
//    _btnBuild.settings().textColor = Qt::white;
//    _btnBuild.setFixedSize(200,28);
//    _btnBuild.setIconSize(QSize(14,14));
//    _btnBuild.setIcon(QIcon(":/images/welcome/load.png"));
//    _btnBuild.setText("Build");
// FIXME   connect(&_btnBuild, &FlarButton::clicked, this, &AndroidWidget::handleBtnBuildClicked);
}

bool AndroidWidget::checkFields()
{
    QString errorMessage;

    if (_txtAppName.text().isEmpty())
        errorMessage += "* Application name cannot be empty.\n";

    if (_txtVersionName.text().isEmpty())
        errorMessage += "* Version name cannot be empty.\n";

    if (_txtPackageName.text().isEmpty())
        errorMessage += "* Package name cannot be empty.\n";
    else if (!_txtPackageName.text().contains(QRegExp
      ("^([A-Za-z]{1}[A-Za-z\\d_]*\\.)*[A-Za-z][A-Za-z\\d_]*$")))
        errorMessage += "* Package name is not valid. Example usage: com.example.myapp\n";

    if (_cmbMinSdk.currentText().split(':').
     first().remove("API ").toInt() > _cmbTargetSdk.
      currentText().split(':').first().remove("API ").toInt())
        errorMessage += "* Target SDK api level cannot be lower than Minimum required SDK api level.\n";

    if (_chkSign.isChecked()) {
        if (_txtKsPath.text().isEmpty())
             errorMessage += "* You have to choose a valid key store file when signing is on.\n";
        else if (_txtKsPw.text().isEmpty())
            errorMessage += "* Key store password cannot be empty.\n";
        else if (_txtKsPw.text().size() < 6)
            errorMessage += "* Key store password is too short.\n";
        else if (_cmbKsAlias.currentText().isEmpty())
            errorMessage += "* Alias name cannot be empty, your key store file is not valid.\n";
        else if (_txtAliasPw.text().isEmpty())
            errorMessage += "* Alias password cannot be empty.\n";
        else if (_txtAliasPw.text().size() < 6)
            errorMessage += "* Alias password is too short.\n";
    }

    if (!errorMessage.isEmpty())
        UtilityFunctions::showMessage(this, tr("Warning"), errorMessage);

    return errorMessage.isEmpty();
}

void AndroidWidget::setTarget(const QString& target)
{
    _lblTitle.setText("Target: " + target);
}

void AndroidWidget::handleBtnBuildClicked()
{
    if (checkFields()) {
        Build::set(TAG_APPNAME, _txtAppName.text());
        Build::set(TAG_VERSIONNAME, _txtVersionName.text());
        Build::set(TAG_VERSIONCODE, _spnVersionCode.value());
        if (_cmbOrientation.currentText() == "Free")
            Build::set(TAG_ORIENTATION, "unspecified");
        else {
            Build::set(TAG_ORIENTATION,
              _cmbOrientation.currentText().toLower());
        }
        Build::setIcon(_txtIconPath.text());

        Build::set(TAG_PACKAGENAME, _txtPackageName.text());
        Build::set(TAG_MINAPI, _cmbMinSdk.currentText().
                   split(':').first().remove("API ").toInt());
        Build::set(TAG_TARGETAPI, _cmbTargetSdk.currentText().
                   split(':').first().remove("API ").toInt());

        Build::clearPermissions();
        for (int i = 0; i < _permissionList.count(); i++) {
            auto permission = _permissionList.item(i)->text();
            Build::addPermission(permission);
        }

        Build::set(TAG_SIGNON, _chkSign.isChecked());
        if (_chkSign.isChecked()) {
            Build::setKeystore(_txtKsPath.text());
            Build::set(TAG_KSPW, _txtKsPw.text());
            Build::set(TAG_ALIASNAME, _cmbKsAlias.currentText());
            Build::set(TAG_ALIASPW, _txtAliasPw.text());
        }

        emit downloadBuild();
    }
}

//TODO: Add "reset form" and critisize side effects
//TODO: Generate certificate
//TODO: Test and check alias and keystore passwords
//TODO: Show aliases
//TODO: Critisize removing build.json when build is done,
// or after closing builds screen