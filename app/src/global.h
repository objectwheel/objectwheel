#ifndef GLOBALS_H
#define GLOBALS_H

#define SIZE_SKIN (QSize(fit(320), fit(662)))
#define SIZE_FORM (QSize(fit(285), fit(535)))
#define SIZE_NONGUI_CONTROL (QSizeF(fit(50), fit(50)))
#define NAME_APP ("Objectwheel")
#define TOOL_KEY ("QURBUEFaQVJMSVlJWiBIQUZJWg")

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

#endif // GLOBALS_H
