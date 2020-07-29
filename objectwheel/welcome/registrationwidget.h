#ifndef REGISTRATIONWIDGET_H
#define REGISTRATIONWIDGET_H

#include <QWidget>

class Switch;
class BulkEdit;
class ButtonSlice;
class WaitingSpinnerWidget;

class RegistrationWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(RegistrationWidget)

public:
    explicit RegistrationWidget(QWidget* parent = nullptr);

private slots:
    void clear();
    void onNextClicked();
    void onSignupSuccessful();
    void onSignupFailure();

signals:
    void back();
    void done(const QString& email);

private:
    BulkEdit* m_bulkEdit;
    Switch* m_termsSwitch;
    ButtonSlice* m_buttons;
    WaitingSpinnerWidget* m_loadingIndicator;
};

#endif // REGISTRATIONWIDGET_H