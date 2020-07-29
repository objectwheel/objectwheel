#ifndef VERIFICATIONWIDGET_H
#define VERIFICATIONWIDGET_H

#include <QLabel>

class BulkEdit;
class ButtonSlice;
class WaitingSpinnerWidget;
class Countdown;

class VerificationWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VerificationWidget)

public:
    explicit VerificationWidget(QWidget* parent = nullptr);

public slots:
    void setEmail(const QString& email);

private slots:
    void clear();
    void onCancelClicked();
    void onResendClicked();
    void onVerifyClicked();
    void onVerifySuccessful();
    void onVerifyFailure();
    void onResendSuccessful();
    void onResendFailure();
    void onDisconnected();
    void onCountdownFinished();

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

#endif // VERIFICATIONWIDGET_H