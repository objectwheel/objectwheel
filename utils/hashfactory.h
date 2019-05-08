#ifndef HASHFACTORY_H
#define HASHFACTORY_H

class QByteArray;

namespace HashFactory {

QByteArray generate();
QByteArray generateSalt();

} // HashFactory

#endif // HASHFACTORY_H