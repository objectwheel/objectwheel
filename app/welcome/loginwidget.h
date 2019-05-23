#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QLabel>

class Switch;
class BulkEdit;
class PushButton;
class ButtonSlice;
class QGridLayout;
class WaitingSpinnerWidget;

class LoginWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(LoginWidget)

public:
    explicit LoginWidget(QWidget* parent = nullptr);

private slots:
    void clear();
    void lock();
    void unlock();
    void onLoginFailure();
    void onLoginSuccessful();
    void onLoginButtonClick();

private:
    void saveRememberMe();
    void clearRememberMe();
    void restoreRememberMe();

signals:
    void done();
    void about();
    void signup();
    void resetPassword();

private:
    QGridLayout* m_layout;
    QLabel* m_logoLabel;
    QLabel* m_loginLabel;
    BulkEdit* m_bulkEdit;
    Switch* m_rememberMeSwitch;
    QLabel* m_rememberMeLabel;
    ButtonSlice* m_buttons;
    PushButton* m_helpButton;
    WaitingSpinnerWidget* m_loadingIndicator;
    QLabel* m_legalLabel;
};

#endif // LOGINWIDGET_H