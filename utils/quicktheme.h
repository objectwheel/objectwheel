#ifndef QUICKTHEME_H
#define QUICKTHEME_H

#include <utils_global.h>

class QString;
namespace QuickTheme
{
UTILS_EXPORT void setTheme(const QString& projectDir, int* version = nullptr);
}

#endif // QUICKTHEME_H
