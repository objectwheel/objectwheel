#ifndef PLATFORM_H
#define PLATFORM_H

#include <QMetaType>

enum Platform {
    Invalid,
    Android,
    iOS,
    macOS,
    Windows,
    Linux,
    RaspberryPi
};

Q_DECLARE_METATYPE(Platform)

#endif // PLATFORM_H
