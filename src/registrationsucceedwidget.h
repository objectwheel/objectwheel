#ifndef REGISTRATIONSUCCEEDWIDGET_H
#define REGISTRATIONSUCCEEDWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class ButtonSlice;
class QMovie;

class RegistrationSucceedWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit RegistrationSucceedWidget(QWidget* parent = nullptr);

    public slots:
        void start();

    signals:
        void done();

    private:
        QVBoxLayout* _layout;
        QMovie* _movie;
        QLabel* _iconLabel;
        QLabel* _titleLabel;
        QLabel* _descriptionLabel;
        ButtonSlice* _buttons;
};

#endif // REGISTRATIONSUCCEEDWIDGET_H