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
    char letter() const;

    bool isValid() const;

private:
    static VpfsVolume* create(const QString& vpdiPath, char letter);

private:
    VpfsVolume(const QString& vpdiPath, char letter);
    ~VpfsVolume();

private:
    const QString& m_path;
    const char m_letter;
};

#endif // VPFSVOLUME_H