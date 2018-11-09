#include <vpfsspace.h>
#include <vpfsvolume.h>
#include <QFileInfo>

QList<VpfsVolume*> VpfsSpace::s_volumes;

const QList<VpfsVolume*>& VpfsSpace::volumes()
{
    return s_volumes;
}

VpfsVolume* VpfsSpace::volume(const QString& vpdiPath)
{
    const QString& canonicalVpdiPath = QFileInfo(vpdiPath).canonicalFilePath();
    for (int i = 0; i < s_volumes.size(); ++i) {
        VpfsVolume* volume = s_volumes.at(i);
        if (volume->path() == canonicalVpdiPath)
            return volume;
    }
    return nullptr;
}

VpfsVolume* VpfsSpace::mount(const QString& vpdiPath)
{
    if (VpfsVolume* found = volume(vpdiPath))
        return found;

    VpfsVolume* vol = VpfsVolume::create(vpdiPath);
    if (vol)
        s_volumes.append(vol);

    return vol;
}

void VpfsSpace::eject(const QString& vpdiPath)
{
    if (VpfsVolume* vol = volume(vpdiPath)) {
        s_volumes.removeOne(vol);
        delete vol;
    }
}

void VpfsSpace::eject(VpfsVolume* volume)
{
    if (s_volumes.contains(volume)) {
        s_volumes.removeOne(volume);
        delete volume;
    }
}