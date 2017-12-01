#include <androidwidget.h>
#include <fit.h>
#include <css.h>
#include <projectmanager.h>
#include <filemanager.h>

AndroidWidget::AndroidWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(this)
    , _appBoxLay(&_appBox)
    , _packageBoxLay(&_packageBox)
    , _permissionsBoxLay(&_permissionsBox)
    , _signingBoxLay(&_signingBox)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);
    setAutoFillBackground(true);

    auto btnLay = new QHBoxLayout;
    btnLay->addWidget(&_btnBack);
    btnLay->addWidget(&_btnBuild);

    _layout.setContentsMargins(fit::fx(20), fit::fx(20), fit::fx(20), fit::fx(20));
    _layout.setSpacing(fit::fx(10));
    _layout.addWidget(&_lblLogo);
    _layout.addWidget(&_lblTitle);
    _layout.addWidget(&_lblMsg);
    _layout.addWidget(&_scrollArea);
    _layout.addLayout(btnLay);
    _layout.setAlignment(&_lblLogo, Qt::AlignHCenter);
    _layout.setAlignment(&_lblTitle, Qt::AlignHCenter);
    _layout.setAlignment(&_lblMsg, Qt::AlignHCenter);
    _layout.setAlignment(&_scrollArea, Qt::AlignHCenter);
    _layout.setAlignment(btnLay, Qt::AlignHCenter);

    _lblLogo.setFixedSize(fit::fx(50), fit::fx(50));
    _lblLogo.setPixmap(QPixmap(":/resources/images/android.png"));
    _lblLogo.setScaledContents(true);

    QFont f;
    f.setPixelSize(fit::fx(28));
    f.setWeight(QFont::ExtraLight);
    QPalette p2(_lblTitle.palette());
    p2.setColor(_lblTitle.foregroundRole(), "#21303c");
    _lblTitle.setFont(f);
    _lblTitle.setPalette(p2);
    _lblTitle.setText("Target: Android");

    f.setPixelSize(fit::fx(17));
    _lblMsg.setFont(f);
    _lblMsg.setPalette(p2);
    _lblMsg.setText("Settings");

    auto scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setLayout(&_scrollAreaLay);
    scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
    scrollAreaWidgetContents->setStyleSheet("#scrollAreaWidgetContents{background: transparent;}");

    QPalette p3(_scrollArea.palette());
    p3.setColor(_scrollArea.backgroundRole(), "#d0d4d7");
    _scrollArea.setPalette(p3);
    _scrollArea.setAutoFillBackground(true);
    _scrollArea.setWidgetResizable(true);
    _scrollArea.setWidget(scrollAreaWidgetContents);
    _scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scrollArea.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _scrollArea.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    _scrollArea.setFixedWidth(fit::fx(600));
    _scrollAreaLay.setSpacing(fit::fx(30));
    _scrollAreaLay.setContentsMargins(fit::fx(20), fit::fx(30), fit::fx(20), fit::fx(30));
    _scrollAreaLay.addWidget(&_appBox);
    _scrollAreaLay.addWidget(&_packageBox);
    _scrollAreaLay.addWidget(&_permissionsBox);
    _scrollAreaLay.addWidget(&_signingBox);

    _appBox.setTitle("Application Settings");
    _appBox.setFixedHeight(fit::fx(280));
    _appBoxLay.addWidget(&_lblAppName, 0, 0);
    _appBoxLay.addWidget(&_txtAppName, 0, 1, 1, 2);
    _appBoxLay.addWidget(&_lblVersionName, 1, 0);
    _appBoxLay.addWidget(&_txtVersionName, 1, 1, 1, 2);
    _appBoxLay.addWidget(&_lblVersionCode, 2, 0);
    _appBoxLay.addWidget(&_spnVersionCode, 2, 1, 1, 2);
    _appBoxLay.addWidget(&_lblOrientation, 3, 0);
    _appBoxLay.addWidget(&_cmbOrientation, 3, 1, 1, 2);
    _appBoxLay.addWidget(&_lblIcon, 4, 0);
    _appBoxLay.addWidget(&_txtIconPath, 4, 1);
    _appBoxLay.addWidget(&_btnIcon, 4, 2);
    _appBoxLay.addWidget(&_picIcon, 5, 1, 1, 2);

    _lblAppName.setText("Application name:");
    _lblVersionCode.setText("Version code:");
    _lblVersionName.setText("Version name:");
    _lblOrientation.setText("Orientation:");
    _lblIcon.setText("Icon:");
    _btnIcon.setText("...");
    _btnIcon.setToolTip("Select icon for your application. It must be at least 256 x 256.\n"
                        "Known image formats such as PNG and JPG are allowed. Your icon may \n"
                        "contain alpha channel. It's better preferring high resolution images.");

    _txtIconPath.setDisabled(true);
    _txtIconPath.setPlaceholderText("[Android Default]");
    _txtIconPath.setToolTip(_btnIcon.toolTip());
    _txtIconPath.setFixedWidth(fit::fx(160));

    _picIcon.setToolTip(_btnIcon.toolTip());
    _picIcon.setFixedSize(fit::fx(QSize{64, 64}));
    _picIcon.setScaledContents(true);
    _picIcon.setPixmap(QPixmap(":/resources/images/android-default.png"));
    _picIcon.setFrameShape(QFrame::StyledPanel);

    _txtAppName.setText("My App");
    _txtAppName.setFixedWidth(fit::fx(200));
    _txtAppName.setToolTip("Your application name.");

    _txtVersionName.setText("1.0");
    _txtVersionName.setFixedWidth(fit::fx(120));
    _txtVersionName.setToolTip("The version number shown to users. This value can be set as a raw string. \n"
                               "The string has no other purpose than to be displayed to users. \n"
                               "The 'Version code' field holds the significant version number used internally.");

    _spnVersionCode.setValue(1);
    _spnVersionCode.setMaximum(99999);
    _spnVersionCode.setFixedWidth(fit::fx(70));
    _spnVersionCode.setToolTip("An internal version number. This number is used only to determine \n"
                               "whether one version is more recent than another, with higher \n"
                               "numbers indicating more recent versions. This is not the version \n"
                               "number shown to users; that number is set by the 'Version name' field. \n"
                               "\n"
                               "The value must be set as an integer, such as '100'. You can define it \n"
                               "however you want, as long as each successive version has a higher number. \n"
                               "For example, it could be a build number. Or you could translate a version \n"
                               "number in 'x.y' format to an integer by encoding the 'x' and 'y' \n"
                               "separately in the lower and upper 16 bits. Or you could simply increase \n"
                               "the number by one each time a new version is released.");

    _cmbOrientation.setFixedWidth(fit::fx(120));
    _cmbOrientation.setIconSize(fit::fx(QSize{14, 14}));
    _cmbOrientation.addItem(QIcon(":/resources/images/free.png"), "Free");
    _cmbOrientation.addItem(QIcon(":/resources/images/landscape.png"), "Landscape");
    _cmbOrientation.addItem(QIcon(":/resources/images/portrait.png"), "Portrait");
    _cmbOrientation.setToolTip("Application orientation. Use Landscape or Portrait to lock your \n"
                               "application orientation; or use Free to leave it unspecified.");

    _packageBox.setTitle("Package Settings");
    _packageBox.setFixedHeight(fit::fx(150));
    _packageBoxLay.addWidget(&_lblPackageName, 0, 0);
    _packageBoxLay.addWidget(&_txtPackageName, 0, 1);
    _packageBoxLay.addWidget(&_lblMinSdk, 1, 0);
    _packageBoxLay.addWidget(&_cmbMinSdk, 1, 1);
    _packageBoxLay.addWidget(&_lblTargetSdk, 2, 0);
    _packageBoxLay.addWidget(&_cmbTargetSdk, 2, 1);

    _lblPackageName.setText("Package name:");
    _lblMinSdk.setText("Minimum required SDK:");
    _lblTargetSdk.setText("Target SDK:");
    _txtPackageName.setText("com.example.myapp");

    _txtPackageName.setFixedWidth(fit::fx(200));
    _txtPackageName.setToolTip("The Android package name, also known as the Google Play ID, the unique \n"
                               "identifier of an application. Please choose a valid package name for \n"
                               "your application (for example, 'org.example.myapp'). \n"
                               "\n"
                               "Packages are usually defined using a hierarchical naming pattern, with \n"
                               "levels in the hierarchy seperated by periods (.) (pronounced 'dot').\n"
                               "\n"
                               "In general, a package name begins with the top level domain name of \n"
                               "the organization and then the organization's domain and then any \n"
                               "subdomains listed in reverse order. The organization can then choose \n"
                               "a spesific name for their package. Package names should be all \n"
                               "lowercase characters whenever possible.\n"
                               "\n"
                               "Completed conventions for disambiguating package names and rules for \n"
                               "naming packages when the Internet domain name cannot be directly \n"
                               "used as a package name are described in section 7.7 of \n"
                               "the Java Language Specification.");

    _cmbMinSdk.setFixedWidth(fit::fx(200));
    _cmbMinSdk.setToolTip("Sets the minimum required version on which this application can be run.");
    _cmbMinSdk.addItem("API 14: Android 4.0, 4.0.1, 4.0.2");
    _cmbMinSdk.addItem("API 15: Android 4.0.3, 4.0.4");
    _cmbMinSdk.addItem("API 16: Android 4.1, 4.1.1");
    _cmbMinSdk.addItem("API 17: Android 4.2, 4.2.2");
    _cmbMinSdk.addItem("API 18: Android 4.3");
    _cmbMinSdk.addItem("API 19: Android 4.4");
    _cmbMinSdk.addItem("API 20: Android 4.4W");
    _cmbMinSdk.addItem("API 21: Android 5.0");
    _cmbMinSdk.addItem("API 22: Android 5.1");
    _cmbMinSdk.addItem("API 23: Android 6.0");
    _cmbMinSdk.setCurrentText("API 17: Android 4.2, 4.2.2");

    _cmbTargetSdk.setFixedWidth(fit::fx(200));
    _cmbTargetSdk.setToolTip("Sets the target SDK. Set this to the highest tested version. This \n"
                             "disables compatibility behavior of the system for your application.");
    _cmbTargetSdk.addItem("API 14: Android 4.0, 4.0.1, 4.0.2");
    _cmbTargetSdk.addItem("API 15: Android 4.0.3, 4.0.4");
    _cmbTargetSdk.addItem("API 16: Android 4.1, 4.1.1");
    _cmbTargetSdk.addItem("API 17: Android 4.2, 4.2.2");
    _cmbTargetSdk.addItem("API 18: Android 4.3");
    _cmbTargetSdk.addItem("API 19: Android 4.4");
    _cmbTargetSdk.addItem("API 20: Android 4.4W");
    _cmbTargetSdk.addItem("API 21: Android 5.0");
    _cmbTargetSdk.addItem("API 22: Android 5.1");
    _cmbTargetSdk.addItem("API 23: Android 6.0");
    _cmbTargetSdk.setCurrentText("API 21: Android 5.0");

    _permissionsBox.setTitle("Permission Settings");
    _permissionsBox.setFixedHeight(fit::fx(200));
    _permissionsBoxLay.addWidget(&_permissionList, 0, 0);
    _permissionsBoxLay.addWidget(&_btnDelPermission, 0, 1);
    _permissionsBoxLay.addWidget(&_cmbPermissions, 1, 0);
    _permissionsBoxLay.addWidget(&_btnAddPermission, 1, 1);
    _permissionsBoxLay.setAlignment(&_btnDelPermission, Qt::AlignBottom);

    QString line;
    QTextStream stream(rdfile(":/resources/other/android-permissions.txt"));
    while (stream.readLineInto(&line)) {
        _cmbPermissions.addItem(line);
    }
    _cmbPermissions.setFixedWidth(fit::fx(400));
    _cmbPermissions.setMaxVisibleItems(15);
    _cmbPermissions.setEditable(true);

    _permissionList.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _permissionList.setFixedWidth(fit::fx(400));
    _permissionList.addItem("android.permission.WRITE_EXTERNAL_STORAGE");
    _permissionList.addItem("android.permission.READ_PHONE_STATE");
    _btnDelPermission.setDisabled(true);
    connect(&_permissionList, &QListWidget::itemSelectionChanged, [&] {
        auto ct = _permissionList.currentItem()->text();
        _btnDelPermission.setEnabled(_permissionList.currentItem() && ct !=
                "android.permission.READ_PHONE_STATE" && ct !=
                "android.permission.WRITE_EXTERNAL_STORAGE");
    });

    connect(&_cmbPermissions, &QComboBox::currentTextChanged, [&] {
        bool found = false;
        auto ct = _cmbPermissions.currentText();
        for (int i = 0; i < _permissionList.count(); i++) {
            if (ct == _permissionList.item(i)->text()) {
                found = true;
                break;
            }
        }
        _btnAddPermission.setDisabled(found);
    });

    connect(&_btnAddPermission, &QPushButton::clicked, [&] {
        _permissionList.addItem(_cmbPermissions.currentText());
        emit _cmbPermissions.currentTextChanged(QString());
    });

    connect(&_btnDelPermission, &QPushButton::clicked, [&] {
        delete _permissionList.takeItem(_permissionList.currentRow());
        emit _cmbPermissions.currentTextChanged(QString());
    });

    QPalette p4(_btnDelPermission.palette());
    p4.setColor(_btnDelPermission.foregroundRole(), "#991223");
    _btnDelPermission.setPalette(p4);
    _permissionList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _btnDelPermission.setText("Delete");
    _btnDelPermission.setIcon(QIcon(":/resources/images/delete.png"));
    _btnAddPermission.setText("Add");
    _btnAddPermission.setIcon(QIcon(":/resources/images/newfile.png"));
    _btnAddPermission.setCursor(Qt::PointingHandCursor);
    _btnDelPermission.setCursor(Qt::PointingHandCursor);
    _btnDelPermission.setToolTip("Delete selected permission from the list.");
    _btnAddPermission.setToolTip("Add permission to the list.");
    _permissionList.setToolTip("Permission list.");
    _cmbPermissions.setToolTip("Select permission to add.");

    _signingBox.setTitle("Signing Settings");
    _signingBox.setFixedHeight(fit::fx(200));
    _signingBoxLay.addWidget(&_lblKsPath, 0, 0);
    _signingBoxLay.addWidget(&_txtKsPath, 0, 1);
    _signingBoxLay.addWidget(&_btnExistingKs, 0, 2);
    _signingBoxLay.addWidget(&_btnNewKs, 1, 1, 1, 2);
    _signingBoxLay.addWidget(&_lblKsPw, 2, 0);
    _signingBoxLay.addWidget(&_txtKsPw, 2, 1, 1, 2);
    _signingBoxLay.addWidget(&_lblKsAlias, 3, 0);
    _signingBoxLay.addWidget(&_txtKsAlias, 3, 1);
    _signingBoxLay.addWidget(&_btnKsAlias, 3, 2);
    _signingBoxLay.addWidget(&_lblKeyPw, 4, 0);
    _signingBoxLay.addWidget(&_txtAliasPw, 4, 1, 1, 2);
    _signingBoxLay.setAlignment(&_btnExistingKs, Qt::AlignRight);

    _btnExistingKs.setCursor(Qt::PointingHandCursor);
    _btnKsAlias.setCursor(Qt::PointingHandCursor);
    _btnNewKs.setCursor(Qt::PointingHandCursor);
    _lblKsPath.setText("Key store path:");
    _btnNewKs.setText("Create new...");
    _btnNewKs.setIcon(QIcon(":/resources/images/newfile.png"));
    _btnExistingKs.setText("...");
    _lblKsPw.setText("Key store password:");
    _lblKsAlias.setText("Alias:");
    _btnKsAlias.setText("...");
    _lblKeyPw.setText("Alias password:");
    _txtKsPath.setPlaceholderText("Select key store file");
    _txtKsPath.setDisabled(true);
    _btnNewKs.setFixedWidth(fit::fx(200));
    _txtKsPath.setFixedWidth(fit::fx(160));
    _txtKsAlias.setFixedWidth(fit::fx(160));
    _txtAliasPw.setFixedWidth(fit::fx(200));
    _txtKsPw.setFixedWidth(fit::fx(200));
    _txtAliasPw.setEchoMode(QLineEdit::Password);
    _txtKsPw.setEchoMode(QLineEdit::Password);

    _txtKsPath.setToolTip("Key store path.");
    _btnExistingKs.setToolTip("Select key store file.");
    _btnNewKs.setToolTip("Generate new key store and signing certificate.");
    _txtKsPw.setToolTip("Type key store password.");
    _txtKsAlias.setToolTip("Type which alias you want to use.");
    _txtAliasPw.setToolTip("Type the related alias's password.");

    _btnBack.setColor("#38A3F6");
    _btnBack.setTextColor(Qt::white);
    _btnBack.setFixedSize(fit::fx(200),fit::fx(28));
    _btnBack.setRadius(fit::fx(7.5));
    _btnBack.setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _btnBack.setIcon(QIcon(":/resources/images/unload.png"));
    _btnBack.setText("Back");
    connect(&_btnBack, &FlatButton::clicked, [&]{
        emit backClicked();
    });

    _btnBuild.setColor("#84BF52");
    _btnBuild.setTextColor(Qt::white);
    _btnBuild.setFixedSize(fit::fx(200),fit::fx(28));
    _btnBuild.setRadius(fit::fx(7.5));
    _btnBuild.setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _btnBuild.setIcon(QIcon(":/resources/images/load.png"));
    _btnBuild.setText("Build");
    //    connect(&_btnBuild, &FlatButton::clicked, [&]{
    //        if (_listWidget.currentItem())
    //            emit platformSelected(Targets
    //              (_listWidget.currentItem()->data(Key).toInt()));
    //    });
}

void AndroidWidget::setTarget(const QString& target)
{
    _lblTitle.setText("Target: " + target);
}
