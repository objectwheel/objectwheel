#include <globalresources.h>
#include <saveutils.h>

GlobalResources* GlobalResources::s_instance = nullptr;
GlobalResources::GlobalResources(const std::function<QString()>& projectDirectoryFunction, QObject* parent)
  : QObject(parent)
  , m_projectDirectoryFunction(projectDirectoryFunction)
{
    s_instance = this;
}

GlobalResources::~GlobalResources()
{
    s_instance = nullptr;
}

GlobalResources* GlobalResources::instance()
{
    return s_instance;
}

QUrl GlobalResources::url() const
{
   return QUrl::fromLocalFile(path());
}

QString GlobalResources::path() const
{
    return SaveUtils::toGlobalDir(m_projectDirectoryFunction());
}
