#ifndef SUPPRESSOR_H
#define SUPPRESSOR_H

#include <functional>

class QString;

namespace Suppressor
{
    void suppress(int msec, const QString& key, const std::function<void()>& function);
}

#endif // SUPPRESSOR_H
