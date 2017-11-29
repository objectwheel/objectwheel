#ifndef GLOBALS_H
#define GLOBALS_H

#define SIZE_SKIN (QSize(fit::fx(320), fit::fx(662)))
#define SIZE_FORM (QSize(fit::fx(285), fit::fx(535)))
#define SIZE_NONGUI_CONTROL (QSizeF(fit::fx(50), fit::fx(50)))
#define NAME_APP ("Objectwheel")
#define TOOL_KEY ("QURBUEFaQVJMSVlJWiBIQUZJWg")
#define SERVER (QString("https://omergoktas.com"))

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
    About,
    Builds
};

enum Skin {
    Invalid,
    PhonePortrait,
    PhoneLandscape,
    Desktop,
    NoSkin
};

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

#endif // GLOBALS_H
