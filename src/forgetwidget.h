#ifndef FORGETWIDGET_H
#define FORGETWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class ButtonSlice;
class BulkEdit;

class ForgetWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ForgetWidget(QWidget* parent = nullptr);

    signals:
        void back();
        void done(const QString& email);

    private:
        QVBoxLayout* _layout;
        QLabel* _iconLabel;
        QLabel* _forgotLabel;
        ButtonSlice* _buttons;
        BulkEdit* _bulkEdit;
};

#endif // FORGETWIDGET_H