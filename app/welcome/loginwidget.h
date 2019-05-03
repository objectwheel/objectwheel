#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QFutureWatcher>
#include <registrationapimanager.h>

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
    void onLoginSuccessful(const RegistrationApiManager::Plans& plan);
    void onLoginFailure();

signals:
    void done();
    void about();
    void signup();
    void forget();
    void busy(const QString& text);

private:
    QFutureWatcher<bool> m_encryptionWatcher;
    QGridLayout* m_layout;
    QLabel* m_logoLabel;
    QLabel* m_loginLabel;
    BulkEdit* m_bulkEdit;
    QWidget* m_autologinWidget;
    QHBoxLayout* m_autologinLayout;
    Switch* m_autologinSwitch;
    QLabel* m_autologinLabel;
    ButtonSlice* m_buttons;
    FlatButton* m_helpButton;
    WaitingSpinnerWidget* m_loadingIndicator;
    QLabel* m_legalLabel;
    QMessageBox* m_helpBox;
};

#endif // LOGINWIDGET_H