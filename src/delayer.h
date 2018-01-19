#ifndef DELAYER_H
#define DELAYER_H

#include <QTimer>
#include <QEventLoop>
#include <functional>

namespace Delayer
{
    void delay(int msec); /* Spin as long as the condition is true */
    void delay(const bool& condition, bool reverse = false, int maxMs = 0, int checkMs = 20);
    void delay(const std::function<bool()>& method, bool reverse = false, int maxMs = 0, int checkMs = 20);
}

#endif // DELAYER_H
