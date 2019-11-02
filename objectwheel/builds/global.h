#ifndef GLOBALS_H
#define GLOBALS_H

#include <QObject>

class OTargets final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OTargets)

public:
    enum Targets {
        android_armeabi_v7a,
        android_x86,
        windows_x86,
        windows_x64,
        linux_x86,
        linux_x64,
        ios,
        macos,
        raspi
    };
    Q_ENUM(Targets)
};

#endif // GLOBALS_H
