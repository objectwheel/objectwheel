#ifndef HASHFACTORY_H
#define HASHFACTORY_H

#include <utils_global.h>

class QByteArray;

namespace HashFactory {

UTILS_EXPORT QByteArray generate();
UTILS_EXPORT QByteArray generateSalt();

} // HashFactory

#endif // HASHFACTORY_H
