#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QFutureWatcher>

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
        void startSession();
        void onSessionStart();
        void onLoginButtonClick();

    signals:
        void done();
        void about();
        void signup();
        void forget();
        void busy(const QString& text);

    private:
        QFutureWatcher<bool> _encryptionWatcher;
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