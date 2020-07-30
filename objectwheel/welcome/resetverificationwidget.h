#ifndef RESETVERIFICATIONWIDGET_H
#define RESETVERIFICATIONWIDGET_H

#include <QLabel>

class BulkEdit;
class ButtonSlice;
class Countdown;
class WaitingSpinnerWidget;

class ResetVerificationWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ResetVerificationWidget)

public:
    explicit ResetVerificationWidget(QWidget* parent = nullptr);

public slots:
    void setEmail(const QString& email);

private slots:
    void onCancelClicked();
    void onCompletePasswordResetClicked();
    void onResendPasswordResetCodeClicked();
    void onCompletePasswordResetSuccessful();
    void onCompletePasswordResetFailure();
    void onResendPasswordResetCodeSuccessful();
    void onResendPasswordResetCodeFailure();
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
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // RESETVERIFICATIONWIDGET_H