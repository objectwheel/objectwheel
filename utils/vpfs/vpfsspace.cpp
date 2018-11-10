#include <vpfsspace.h>
#include <vpfsvolume.h>
#include <QFileInfo>

QList<VpfsVolume*> VpfsSpace::s_volumes;

const QList<VpfsVolume*>& VpfsSpace::volumes()
{
    return s_volumes;
}

VpfsVolume* VpfsSpace::volume(char letter)
{
    for (int i = 0; i < s_volumes.size(); ++i) {
        VpfsVolume* volume = s_volumes[i];
        QChar l(volume->letter());
        if (l.toLower().toLatin1() == letter || l.toUpper().toLatin1() == letter)
            return volume;
    }
    return nullptr;
}

VpfsVolume* VpfsSpace::volume(const QString& vpdiPath)
{
    const QString& canonicalVpdiPath = QFileInfo(vpdiPath).canonicalFilePath();
    for (int i = 0; i < s_volumes.size(); ++i) {
        VpfsVolume* volume = s_volumes[i];
        if (volume->path().compare(canonicalVpdiPath, Qt::CaseInsensitive) == 0)
            return volume;
    }
    return nullptr;
}

VpfsVolume* VpfsSpace::mount(const QString& vpdiPath, char letter)
{
    if (VpfsVolume* found = volume(vpdiPath))
        return found;

    if (letter == 0)
        letter = newUniqueLetter();

    VpfsVolume* vol = VpfsVolume::create(vpdiPath, letter);
    if (vol)
        s_volumes.append(vol);

    return vol;
}

void VpfsSpace::eject(char letter)
{
    if (VpfsVolume* vol = volume(letter)) {
        s_volumes.removeOne(vol);
        delete vol;
    }
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

char VpfsSpace::newUniqueLetter()
{
    static const auto& letterExists = [] (char letter) -> bool {
        for (const VpfsVolume* volume : s_volumes) {
            if (volume->letter() == letter)
                return true;
        }
        return false;
    };

    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (!letterExists(letter) && !letterExists(letter + 32))
            return letter;
    }

    qWarning("You reached the maximum number of mounted volumes.");

    return 0;
}