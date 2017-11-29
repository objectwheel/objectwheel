#include <androidwidget.h>
#include <fit.h>
#include <css.h>
#include <QScrollBar>

AndroidWidget::AndroidWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(this)
    , _appBoxLay(&_appBox)
    , _packageBoxLay(&_packageBox)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);
    setAutoFillBackground(true);

    auto& btnLay = *new QHBoxLayout;
    btnLay.addWidget(&_btnBack);
    btnLay.addWidget(&_btnBuild);

    _layout.setContentsMargins(fit::fx(20), fit::fx(20), fit::fx(20), fit::fx(20));
    _layout.setSpacing(fit::fx(10));
    _layout.addWidget(&_lblLogo);
    _layout.addWidget(&_lblTitle);
    _layout.addWidget(&_lblMsg);
    _layout.addWidget(&_scrollArea);
    _layout.addLayout(&btnLay);
    _layout.setAlignment(&_lblLogo, Qt::AlignHCenter);
    _layout.setAlignment(&_lblTitle, Qt::AlignHCenter);
    _layout.setAlignment(&_lblMsg, Qt::AlignHCenter);
    _layout.setAlignment(&_scrollArea, Qt::AlignHCenter);
    _layout.setAlignment(&btnLay, Qt::AlignHCenter);

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

    QPalette p3;
    p3.setColor(QPalette::Base, "#e2e6e9");
    p3.setColor(QPalette::Window, "#d8dcdf");
    _scrollArea.setPalette(p3);
    _scrollArea.setAutoFillBackground(true);
    _scrollArea.setWidgetResizable(true);
    auto scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setGeometry(QRect(0, 0, 669, 471));
    scrollAreaWidgetContents->setPalette(p3);
    scrollAreaWidgetContents->setLayout(&_scrollAreaLay);
    _scrollArea.setWidget(scrollAreaWidgetContents);
    _scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scrollArea.verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    _scrollArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _scrollArea.setFixedWidth(fit::fx(500));
    _scrollAreaLay.setSpacing(fit::fx(30));
    _scrollAreaLay.setContentsMargins(fit::fx(30), fit::fx(30), fit::fx(30), fit::fx(30));
    _scrollAreaLay.addWidget(&_appBox);
    _scrollAreaLay.addWidget(&_packageBox);

    _appBox.setTitle("Application Settings");
    _appBox.setFixedHeight(fit::fx(200));
    _appBoxLay.addWidget(&_lblAppName, 0, 0);
    _appBoxLay.addWidget(&_txtAppName, 0, 1);
    _appBoxLay.addWidget(&_lblVersionCode, 1, 0);
    _appBoxLay.addWidget(&_spnVersionCode, 1, 1);
    _appBoxLay.addWidget(&_lblVersionName, 2, 0);
    _appBoxLay.addWidget(&_txtVersionName, 2, 1);
    _appBoxLay.addWidget(&_lblOrientation, 3, 0);
    _appBoxLay.addWidget(&_cmbOrientation, 3, 1);
    _appBoxLay.addWidget(&_lblIcon, 4, 0);

    _lblAppName.setText("Application name: ");
    _lblVersionCode.setText("Version code: ");
    _lblVersionName.setText("Version name: ");
    _lblOrientation.setText("Orientation: ");
    _lblIcon.setText("Icon: ");

    _packageBox.setTitle("Package Settings");
    _packageBox.setFixedHeight(fit::fx(200));
    _packageBoxLay.addWidget(&_lblPackageName, 0, 0);
    _packageBoxLay.addWidget(&_txtPackageName, 0, 1);
    _packageBoxLay.addWidget(&_lblMinSdk, 1, 0);
    _packageBoxLay.addWidget(&_cmbMinSdk, 1, 1);
    _packageBoxLay.addWidget(&_lblTargetSdk, 2, 0);
    _packageBoxLay.addWidget(&_cmbTargetSdk, 2, 1);
    _packageBoxLay.addWidget(&_lblPermissions, 3, 0);

    _lblPackageName.setText("Package name: ");
    _lblMinSdk.setText("Minimum required SDK: ");
    _lblTargetSdk.setText("Target SDK: ");
    _lblPermissions.setText("Permissions: ");


    _btnBack.setColor("#38A3F6");
    _btnBack.setTextColor(Qt::white);
    _btnBack.setFixedSize(fit::fx(200),fit::fx(28));
    _btnBack.setRadius(fit::fx(7.5));
    _btnBack.setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _btnBack.setIcon(QIcon(":/resources/images/load.png"));
    _btnBack.setText("Back");
    connect(&_btnBack, &FlatButton::clicked, [&]{
        emit backClicked();
    });


    _btnBuild.setColor("#81B251");
    _btnBuild.setTextColor(Qt::white);
    _btnBuild.setFixedSize(fit::fx(200),fit::fx(28));
    _btnBuild.setRadius(fit::fx(7.5));
    _btnBuild.setIconSize(QSize(fit::fx(14),fit::fx(14)));
    _btnBuild.setIcon(QIcon(":/resources/images/unload.png"));
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
