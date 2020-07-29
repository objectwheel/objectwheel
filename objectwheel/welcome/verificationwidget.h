#ifndef VERIFICATIONWIDGET_H
#define VERIFICATIONWIDGET_H

#include <QLabel>

class BulkEdit;
class ButtonSlice;
class Countdown;
class WaitingSpinnerWidget;

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
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // VERIFICATIONWIDGET_H