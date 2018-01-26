#ifndef FORGETWIDGET_H
#define FORGETWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class ButtonSlice;
class BulkEdit;
class WaitingSpinnerWidget;

class ForgetWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ForgetWidget(QWidget* parent = nullptr);

    private slots:
        void clear();
        void lock();
        void unlock();
        void onNextClicked();

    signals:
        void back();
        void done(const QString& email);

    private:
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _forgotLabel;
        ButtonSlice* _buttons;
        BulkEdit* _bulkEdit;
        WaitingSpinnerWidget* _loadingIndicator;
};

#endif // FORGETWIDGET_H