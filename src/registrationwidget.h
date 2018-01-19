#ifndef REGISTRATIONWIDGET_H
#define REGISTRATIONWIDGET_H

#include <QWidget>

class QLabel;
class Switch;
class BulkEdit;
class ButtonSlice;
class QVBoxLayout;
class QHBoxLayout;
class WaitingSpinnerWidget;

class RegistrationWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit RegistrationWidget(QWidget *parent = nullptr);

    private:
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _signupLabel;
        BulkEdit* _bulkEdit;
        QWidget* _termsWidget;
        QHBoxLayout* _termsLayout;
        Switch* _termsSwitch;
        QLabel* _termsLabel;
        ButtonSlice* _buttons;
        WaitingSpinnerWidget* _loadingIndicator;
        QLabel* _legalLabel;

};

#endif // REGISTRATIONWIDGET_H