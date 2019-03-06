#include <vpfsvolume.h>
#include <QFileInfo>

namespace {

inline bool letterValid(char letter)
{
    return (letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z');
}
}

VpfsVolume::VpfsVolume(const QString& vpdiPath, char letter) : m_path(vpdiPath)
  , m_letter(letter)
{
}

VpfsVolume::~VpfsVolume()
{
}

VpfsVolume* VpfsVolume::create(const QString& vpdiPath, char letter)
{
    if (vpdiPath.isEmpty())
        return nullptr;

    if (!letterValid(letter))
        return nullptr;

    const QString& canonicalVpdiPath = QFileInfo(vpdiPath).canonicalFilePath();

    if (canonicalVpdiPath.isEmpty())
        return nullptr;

    if (!QFileInfo::exists(canonicalVpdiPath))
        return nullptr;

    if (!QFileInfo(canonicalVpdiPath).isReadable())
        return nullptr;

    VpfsVolume* volume = new VpfsVolume(canonicalVpdiPath, letter);

    if (volume->isValid())
        return volume;

    delete volume;
    return nullptr;
}

const QString& VpfsVolume::path() const
{
    return m_path;
}

char VpfsVolume::letter() const
{
    return m_letter;
}

bool VpfsVolume::isValid() const
{
    return false;
}
