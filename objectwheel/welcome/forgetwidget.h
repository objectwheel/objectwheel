#ifndef FORGETWIDGET_H
#define FORGETWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class ButtonSlice;
class BulkEdit;
class WaitingSpinnerWidget;

class ForgetWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ForgetWidget)

public:
    explicit ForgetWidget(QWidget* parent = nullptr);

private slots:
    void clear();
    void lock();
    void unlock();
    void onNextClicked();
    void onResetPasswordSuccessful();
    void onResetPasswordFailure();

signals:
    void back();
    void done(const QString& email);

private:
    QVBoxLayout* m_layout;
    QLabel* m_iconLabel;
    QLabel* m_forgotLabel;
    ButtonSlice* m_buttons;
    BulkEdit* m_bulkEdit;
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // FORGETWIDGET_H