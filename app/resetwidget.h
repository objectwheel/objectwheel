#ifndef RESETWIDGET_H
#define RESETWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class BulkEdit;
class ButtonSlice;
class WaitingSpinnerWidget;
class Countdown;

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResetWidget(QWidget* parent = nullptr);

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


#endif // RESETWIDGET_H