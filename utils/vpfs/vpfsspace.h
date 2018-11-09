#ifndef VPFSSPACE_H
#define VPFSSPACE_H

#include <QList>

class VpfsVolume;
class VpfsSpace final
{
    VpfsSpace() = delete;
    VpfsSpace(const VpfsSpace&) = delete;
    VpfsSpace &operator=(const VpfsSpace&) = delete;

public:
    static const QList<VpfsVolume*>& volumes();

    static VpfsVolume* volume(const QString& vpdiPath);
    static VpfsVolume* mount(const QString& vpdiPath, char letter = 0);

    static void eject(const QString& vpdiPath);
    static void eject(VpfsVolume* volume);

private:
    static char newUniqueLetter();

private:
    static QList<VpfsVolume*> s_volumes;
};

#endif // VPFSSPACE_H