#ifndef ANDROIDPLATFORMWIDGET_H
#define ANDROIDPLATFORMWIDGET_H

#include <QtWidgets>

class AndroidPlatformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AndroidPlatformWidget(QWidget *parent = nullptr);
    bool checkFields();

public slots:
    void setTarget(const QString&target);

signals:
    void backClicked();
    void downloadBuild();

private slots:
    void handleBtnBuildClicked();

private:
    QVBoxLayout _layout;
    QLabel _lblLogo;
    QLabel _lblTitle;
    QLabel _lblMsg;
//  FIXME  FlarButton _btnBack;
//    FlarButton _btnBuild;
    QScrollArea _scrollArea;
    QVBoxLayout _scrollAreaLay;

    QGroupBox _appBox;
    QGridLayout _appBoxLay;
    QLabel _lblAppName;
    QLineEdit _txtAppName;
    QLabel _lblVersionCode;
    QSpinBox _spnVersionCode;
    QLabel _lblVersionName;
    QLineEdit _txtVersionName;
    QLabel _lblOrientation;
    QComboBox _cmbOrientation;
    QLabel _lblIcon;
    QLineEdit _txtIconPath;
    QToolButton _btnIcon;
    QLabel _picIcon;

    QGroupBox _packageBox;
    QGridLayout _packageBoxLay;
    QLabel _lblPackageName;
    QLineEdit _txtPackageName;
    QLabel _lblMinSdk;
    QComboBox _cmbMinSdk;
    QLabel _lblTargetSdk;
    QComboBox _cmbTargetSdk;

    QGroupBox _permissionsBox;
    QGridLayout _permissionsBoxLay;
    QListWidget _permissionList;
    QPushButton _btnAddPermission;
    QPushButton _btnDelPermission;
    QComboBox _cmbPermissions;

    QGroupBox _signingBox;
    QGridLayout _signingBoxLay;
    QLabel _lblKsPath;
    QLineEdit _txtKsPath;
    QToolButton _btnExistingKs;
    QPushButton _btnNewKs;
    QLabel _lblKsPw;
    QLineEdit _txtKsPw;
    QLabel _lblKsAlias;
    QComboBox _cmbKsAlias;
    QLabel _lblKeyPw;
    QLineEdit _txtAliasPw;
    QLabel _lblSign;
    QCheckBox _chkSign;
};

#endif // ANDROIDPLATFORMWIDGET_H
