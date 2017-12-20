#ifndef GLOBALS_H
#define GLOBALS_H

#include <QObject>

#define SIZE_SKIN (QSize(fit::fx(320), fit::fx(662)))
#define SIZE_FORM (QSize(fit::fx(285), fit::fx(535)))
#define SIZE_NONGUI_CONTROL (QSizeF(fit::fx(50), fit::fx(50)))
#define TOOL_KEY ("QURBUEFaQVJMSVlJWiBIQUZJWg")
#define TEXT_LEGAL (QString("<p><b>© 2015 - 2017 %1 All Rights Reserved.</b></p>").arg(APP_CORP))
#define TEXT_VERSION (QString("<p><b>version</b> v%1 <b>hash</b> %2 <b>date</b><br> %3 <br></p>")\
    .arg(APP_VER).arg(APP_GITHASH).arg(APP_GITDATE))

enum DesignMode {
    ControlGui,
    FormGui,
    CodeEdit
};

enum Screen {
    Progress,
    Studio,
    Projects,
    Login,
    Builds
};

enum Skin {
    Invalid,
    PhonePortrait,
    PhoneLandscape,
    Desktop,
    NoSkin
};

class OTargets : public QObject {
        Q_OBJECT
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
