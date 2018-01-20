#ifndef VERIFICATIONWIDGET_H
#define VERIFICATIONWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class BulkEdit;
class ButtonSlice;
class WaitingSpinnerWidget;
class Countdown;

class VerificationWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit VerificationWidget(QWidget *parent = nullptr);

    public slots:
        void setEmail(const QString& email);

    private slots:
        void clear();
        void lock();
        void unlock();
        void onCancelClicked();
        void onResendClicked();
        void onVerifyClicked();

    signals:
        void done();
        void cancel();

    private:
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _countdownLabel;
        Countdown* _countdown;
        QLabel* _verificationLabel;
        QLabel* _emailLabel;
        BulkEdit* _bulkEdit;
        QWidget* _termsWidget;
        ButtonSlice* _buttons;
        WaitingSpinnerWidget* _loadingIndicator;
        QLabel* _legalLabel;
};

#endif // VERIFICATIONWIDGET_H