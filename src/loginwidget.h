#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class QLabel;
class Switch;
class BulkEdit;
class QGridLayout;
class QHBoxLayout;

class LoginWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LoginWidget(QWidget *parent = nullptr);

    private:
        QGridLayout* _layout;
        QLabel* _logoLabel;
        QLabel* _loginLabel;
        BulkEdit* _bulkEdit;
        QWidget* _autologinWidget;
        QHBoxLayout* _autologinLayout;
        Switch* _autologinSwitch;
        QLabel* _autologinLabel;
        QLabel* _legalLabel;
};

#endif // LOGINWIDGET_H