#ifndef RESETVERIFICATIONWIDGET_H
#define RESETVERIFICATIONWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class BulkEdit;
class ButtonSlice;
class WaitingSpinnerWidget;
class Countdown;

class ResetVerificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResetVerificationWidget(QWidget* parent = nullptr);

public slots:
    void setEmail(const QString& email);

private slots:
    void clear();
    void lock();
    void unlock();
    void onCancelClicked();
    void onApplyClicked();

signals:
    void done();
    void cancel();

private:
    QVBoxLayout* _layout;
    QLabel* _iconLabel;
    QLabel* _countdownLabel;
    Countdown* _countdown;
    QLabel* _resetLabel;
    QLabel* _emailLabel;
    BulkEdit* _bulkEdit;
    ButtonSlice* _buttons;
    WaitingSpinnerWidget* _loadingIndicator;
};

#endif // RESETVERIFICATIONWIDGET_H