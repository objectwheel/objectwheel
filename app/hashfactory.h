#ifndef HASHFACTORY_H
#define HASHFACTORY_H

#include <QString>

class HashFactory
{
        Q_DISABLE_COPY(HashFactory)

    public:
        static QString generate();

    private:
        HashFactory() {}
};

#endif // HASHFACTORY_H