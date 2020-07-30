#ifndef SIGNUPWIDGET_H
#define SIGNUPWIDGET_H

#include <QWidget>

class Switch;
class BulkEdit;
class ButtonSlice;
class WaitingSpinnerWidget;

class SignupWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SignupWidget)

public:
    explicit SignupWidget(QWidget* parent = nullptr);

private slots:
    void onNextClicked();
    void onSignupSuccessful();
    void onSignupFailure();
    void onServerDisconnected();

signals:
    void back();
    void done(const QString& email);

private:
    BulkEdit* m_bulkEdit;
    Switch* m_termsSwitch;
    ButtonSlice* m_buttons;
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // SIGNUPWIDGET_H