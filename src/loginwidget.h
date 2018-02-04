#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class QLabel;
class Switch;
class BulkEdit;
class FlatButton;
class ButtonSlice;
class QGridLayout;
class QHBoxLayout;
class WaitingSpinnerWidget;
class QMessageBox;

class LoginWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LoginWidget(QWidget *parent = nullptr);

    private slots:
        void lock();
        void unlock();
        void clear();
        void onLoginButtonClicked();

    signals:
        void about();
        void signup();
        void forget();
        void done(const QString& email, const QString& password, const QString& plan);

    private:
        QGridLayout* _layout;
        QLabel* _logoLabel;
        QLabel* _loginLabel;
        BulkEdit* _bulkEdit;
        QWidget* _autologinWidget;
        QHBoxLayout* _autologinLayout;
        Switch* _autologinSwitch;
        QLabel* _autologinLabel;
        ButtonSlice* _buttons;
        FlatButton* _helpButton;
        WaitingSpinnerWidget* _loadingIndicator;
        QLabel* _legalLabel;
        QMessageBox* _helpBox;
};

#endif // LOGINWIDGET_H