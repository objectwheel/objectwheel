#include <delayer.h>
#include <QTimer>
#include <QEventLoop>

void Delayer::delay(const int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, [&]{ loop.quit(); });
    loop.exec();
}
