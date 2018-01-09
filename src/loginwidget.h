#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class LoginWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LoginWidget(QWidget *parent = nullptr);

    protected:
        void paintEvent(QPaintEvent *event) override;

};

#endif // LOGINWIDGET_H