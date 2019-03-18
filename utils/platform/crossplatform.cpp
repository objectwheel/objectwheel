#include <crossplatform.h>
#include <QSysInfo>

#if defined(Q_OS_MACOS)
#include <deviceinfo.h>
#endif

namespace CrossPlatform {

QString deviceName()
{
#if defined(Q_OS_MACOS)
    static const QString deviceName = Macos::DeviceInfo::deviceName();
#else
    static const QString deviceName = QSysInfo::machineHostName();
#endif
    return deviceName;
}
}
