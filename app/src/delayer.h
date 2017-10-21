#ifndef DELAYER_H
#define DELAYER_H

#include <QTimer>
#include <QEventLoop>

namespace Delayer
{
    void delay(const int msec);
    void delay(const bool& condition, bool reverse = false, const int maxMs = 0, const int checkMs = 20);
    void delay(bool (*const method)(), bool reverse = false, const int maxMs = 0, const int checkMs = 20);
    template<typename T>
    void delay(const void* object, bool (T::*const method)() const, bool reverse = false, const int maxMs = 0, const int checkMs = 20)
    {
        QEventLoop loop;
        QTimer timer, timer_2;
        QObject::connect(&timer, &QTimer::timeout, [&] { if (reverse ? ((((T*)object)->*method)()) : !((((T*)object)->*method)())) loop.quit(); });
        QObject::connect(&timer_2, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(checkMs);
        if (maxMs > 0) timer_2.start(maxMs);
        loop.exec();
    }
    template<typename T>
    void delay(const void* object, bool (T::*const method)(), bool reverse = false, const int maxMs = 0, const int checkMs = 20)
    {
        QEventLoop loop;
        QTimer timer, timer_2;
        QObject::connect(&timer, &QTimer::timeout, [&] { if (reverse ? ((((T*)object)->*method)()) : !((((T*)object)->*method)())) loop.quit(); });
        QObject::connect(&timer_2, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(checkMs);
        if (maxMs > 0) timer_2.start(maxMs);
        loop.exec();
    }
}

#endif // DELAYER_H
