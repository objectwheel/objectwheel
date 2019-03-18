#include <deviceinfo.h>
#include <Foundation/Foundation.h>

namespace Macos {

namespace DeviceInfo {

QString deviceName()
{
    return QString::fromNSString([[NSHost currentHost] localizedName]);
}
}
}
