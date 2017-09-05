#include <delayer.h>

void Delayer::delay(const int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, [&]{ loop.quit(); });
    loop.exec();
}

void Delayer::delay(const bool& condition, bool reverse, const int maxMs, const int checkMs)
{
    QEventLoop loop;
    QTimer timer, timer_2;
    QObject::connect(&timer, &QTimer::timeout, [&] { if (reverse ? condition : !condition) loop.quit(); });
    QObject::connect(&timer_2, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(checkMs);
    if (maxMs > 0) timer_2.start(maxMs);
    loop.exec();
}

void Delayer::delay(bool (* const method)(), bool reverse, const int maxMs, const int checkMs)
{
    QEventLoop loop;
    QTimer timer, timer_2;
    QObject::connect(&timer, &QTimer::timeout, [&] {
        if (reverse ? ((*method)()) : !((*method)())) loop.quit(); });
    QObject::connect(&timer_2, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(checkMs);
    if (maxMs > 0) timer_2.start(maxMs);
    loop.exec();
}
