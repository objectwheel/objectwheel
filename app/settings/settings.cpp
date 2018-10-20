#include <settings.h>

Settings::Settings(QObject* parent) : Settings({}, parent)
{
}

Settings::Settings(const QString& group, QObject* parent) : QObject(parent)
  , m_group(group)
{
}

const QString& Settings::group() const
{
    return m_group;
}
