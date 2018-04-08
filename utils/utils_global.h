#ifndef UTILS_GLOBAL_H
#define UTILS_GLOBAL_H

#include <qglobal.h>

#if defined(UTILS_LIBRARY)
#  define UTILS_EXPORT Q_DECL_EXPORT
#elif defined(UTILS_BUILD_STATIC_LIB)
#  define UTILS_EXPORT
#else
#  define UTILS_EXPORT Q_DECL_IMPORT
#endif

#endif // UTILS_GLOBAL_H
