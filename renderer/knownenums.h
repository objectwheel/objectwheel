#ifndef KNOWNENUMS_H
#define KNOWNENUMS_H

#include <renderinfo.h>

namespace KnownEnums {

Enum extractEnum(const QMetaProperty& property, const QMetaEnum& me, const QObject* object);

bool isKnownEnum(const QString& identifier);

Enum knownEnum(const QMetaProperty& property, const QObject* object, const QString& identifier);

}

#endif // KNOWNENUMS_H
