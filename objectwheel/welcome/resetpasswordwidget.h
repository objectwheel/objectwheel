#ifndef RESETPASSWORDWIDGET_H
#define RESETPASSWORDWIDGET_H

#include <QWidget>

class BulkEdit;
class ButtonSlice;
class BusyIndicatorWidget;

class ResetPasswordWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ResetPasswordWidget)

public:
    explicit ResetPasswordWidget(QWidget* parent = nullptr);

private slots:
    void onNextClicked();
    void onResetPasswordSuccessful();
    void onResetPasswordFailure();
    void onServerDisconnected();

signals:
    void back();
    void done(const QString& email);

private:
    BulkEdit* m_bulkEdit;
    ButtonSlice* m_buttons;
    BusyIndicatorWidget* m_busyIndicator;
};

#endif // RESETPASSWORDWIDGET_H