#include <settings.h>

Settings::Settings(const QString& group) : m_group(group)
{
}

const QString& Settings::group() const
{
    return m_group;
}
