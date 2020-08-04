#ifndef SIGNUPVERIFICATIONWIDGET_H
#define SIGNUPVERIFICATIONWIDGET_H

#include <QLabel>

class BulkEdit;
class ButtonSlice;
class Countdown;
class BusyIndicatorWidget;

class SignupVerificationWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SignupVerificationWidget)

public:
    explicit SignupVerificationWidget(QWidget* parent = nullptr);

public slots:
    void setEmail(const QString& email);

private slots:
    void onCancelClicked();
    void onCompleteSignupClicked();
    void onResendSignupCodeClicked();
    void onCompleteSignupSuccessful();
    void onCompleteSignupFailure();
    void onResendSignupCodeSuccessful();
    void onResendSignupCodeFailure();
    void onCountdownFinished();
    void onServerDisconnected();

signals:
    void done();
    void cancel();

private:
    Countdown* m_countdown;
    QLabel* m_emailLabel;
    BulkEdit* m_bulkEdit;
    ButtonSlice* m_buttons;
    BusyIndicatorWidget* m_busyIndicator;
};

#endif // SIGNUPVERIFICATIONWIDGET_H