#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class Switch;
class BulkEdit;
class QPushButton;
class ButtonSlice;
class WaitingSpinnerWidget;

class LoginWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(LoginWidget)

public:
    explicit LoginWidget(QWidget* parent = nullptr);

private slots:
    void onHelpButtonClicked();
    void onLoginButtonClicked();
    void onLoginSuccessful();
    void onLoginFailure();
    void onServerDisconnected();

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
    BulkEdit* m_bulkEdit;
    Switch* m_rememberMeSwitch;
    ButtonSlice* m_buttons;
    QPushButton* m_helpButton;
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // LOGINWIDGET_H