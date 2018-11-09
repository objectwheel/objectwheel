#include <vpfsvolume.h>
#include <QFileInfo>

VpfsVolume::VpfsVolume(const QString& vpdiPath) : m_path(vpdiPath)
{

}

VpfsVolume::~VpfsVolume()
{

}

VpfsVolume* VpfsVolume::create(const QString& vpdiPath)
{
    if (vpdiPath.isEmpty())
        return nullptr;

    const QString& canonicalVpdiPath = QFileInfo(vpdiPath).canonicalFilePath();

    if (canonicalVpdiPath.isEmpty())
        return nullptr;

    if (!QFileInfo::exists(canonicalVpdiPath))
        return nullptr;

    if (!QFileInfo(canonicalVpdiPath).isReadable())
        return nullptr;

    VpfsVolume* volume = new VpfsVolume(canonicalVpdiPath);

    if (volume->isValid())
        return volume;

    delete volume;
    return nullptr;
}

const QString& VpfsVolume::path() const
{
    return m_path;
}

bool VpfsVolume::isValid() const
{

}
