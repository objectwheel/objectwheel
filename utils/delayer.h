#ifndef DELAYER_H
#define DELAYER_H

#include <utils_global.h>
#include <QTimer>
#include <QEventLoop>
#include <functional>

namespace Delayer {
UTILS_EXPORT void delay(int msec); /* Spin as long as the condition is true */
UTILS_EXPORT void delay(const bool* condition, bool reverse = false, int maxMs = 0, int checkMs = 20);
UTILS_EXPORT void delay(const std::function<bool()>& method, bool reverse = false, int maxMs = 0, int checkMs = 20);
}

#endif // DELAYER_H
