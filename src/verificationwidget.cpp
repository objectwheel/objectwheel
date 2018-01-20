#include <verificationwidget.h>
#include <countdown.h>

VerificationWidget::VerificationWidget(QWidget *parent) : QWidget(parent)
{
    auto cd = new Countdown(this);
    cd->move(200, 200);
    cd->move(300, 200);
}
