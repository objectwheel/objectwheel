#ifndef GLOBALS_H
#define GLOBALS_H

#define SIZE_SKIN (fit(QSize(320, 662)))
#define SIZE_FORM (fit(QSize(285, 535)))

enum Screen {
    PROGRESS,
    STUDIO,
    PROJECTS,
    LOGIN,
    ABOUT,
    BUILDS
};

enum OutputPane {
    ISSUES,
    SEARCH,
    APPOUTPUT
};

enum Skin {
    Invalid,
    PhonePortrait,
    PhoneLandscape,
    Desktop,
    NoSkin
};

#endif // GLOBALS_H
