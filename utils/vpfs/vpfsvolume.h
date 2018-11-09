#ifndef VPFSVOLUME_H
#define VPFSVOLUME_H

#include <QObject>

class VpfsVolume final
{
    // To make its construction and destruction restricted to VpfsSpace
    friend class VpfsSpace;

    VpfsVolume(const VpfsVolume&) = delete;
    VpfsVolume &operator=(const VpfsVolume&) = delete;

public:
    const QString& path() const;

    bool isValid() const;

private:
    static VpfsVolume* create(const QString& vpdiPath);

private:
    VpfsVolume(const QString& vpdiPath);
    ~VpfsVolume();

private:
    const QString& m_path;
};

#endif // VPFSVOLUME_H