#ifndef SERIALIZEENUM_H
#define SERIALIZEENUM_H

#include <QDataStream>

template <typename Enum, typename = std::enable_if_t<std::is_enum<Enum>::value>>
inline QDataStream& operator>> (QDataStream& in, Enum& value)
{
    std::underlying_type_t<Enum> temporary;
    in >> temporary;
    value = static_cast<Enum>(temporary);
    return in;
}

template <typename Enum, typename = std::enable_if_t<std::is_enum<Enum>::value>>
inline QDataStream& operator<< (QDataStream& out, Enum value)
{
    out << static_cast<std::underlying_type_t<Enum>>(value);
    return out;
}

#endif // SERIALIZEENUM_H
