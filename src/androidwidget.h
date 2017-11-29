#ifndef ANDROIDWIDGET_H
#define ANDROIDWIDGET_H

#include <flatbutton.h>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QScrollArea>

class AndroidWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit AndroidWidget(QWidget *parent = nullptr);

    public slots:
        void setTarget(const QString&target);

    signals:
        void backClicked();

    private:
        QVBoxLayout _layout;
        QLabel _lblLogo;
        QLabel _lblTitle;
        QLabel _lblMsg;
        FlatButton _btnBack;
        FlatButton _btnBuild;
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

        QGroupBox _packageBox;
        QGridLayout _packageBoxLay;
        QLabel _lblPackageName;
        QLineEdit _txtPackageName;
        QLabel _lblMinSdk;
        QComboBox _cmbMinSdk;
        QLabel _lblTargetSdk;
        QComboBox _cmbTargetSdk;

        QGroupBox _permissionsBox;
//        QGridLayout _appBoxLay;
        QLabel _lblPermissions;

        QGroupBox _crtificateBox;
//        QGridLayout _appBoxLay;

};

#endif // ANDROIDWIDGET_H
