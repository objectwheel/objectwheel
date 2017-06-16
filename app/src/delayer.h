#ifndef DELAYER_H
#define DELAYER_H

#include <QTimer>
#include <QEventLoop>

namespace Delayer
{
    void delay(const int msec);
    void delay(const bool& condition, const int maxMs = 0, const int checkMs = 20);
    void delay(bool (*const method)(), const int maxMs = 0, const int checkMs = 20);
    template<typename T>
    void delay(void* object, bool (T::*const method)() const, const int maxMs = 0, const int checkMs = 20)
    {       
        QEventLoop loop;
        QTimer timer, timer_2;
        QObject::connect(&timer, &QTimer::timeout, [&] { if (!((((T*)object)->*method)())) loop.quit(); });
        QObject::connect(&timer_2, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(checkMs);
        if (maxMs > 0) timer_2.start(maxMs);
        loop.exec();
    }
    template<typename T>
    void delay(void* object, bool (T::*const method)(), const int maxMs = 0, const int checkMs = 20)
    {
        QEventLoop loop;
        QTimer timer, timer_2;
        QObject::connect(&timer, &QTimer::timeout, [&] { if (!((((T*)object)->*method)())) loop.quit(); });
        QObject::connect(&timer_2, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(checkMs);
        if (maxMs > 0) timer_2.start(maxMs);
        loop.exec();
    }
}

#endif // DELAYER_H
