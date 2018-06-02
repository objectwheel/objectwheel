#ifndef APPFONTSETTINGS_H
#define APPFONTSETTINGS_H

#include <QtGlobal>

class AppFontSettings final {
    Q_DISABLE_COPY(AppFontSettings)

public:
    static void apply();

private:
    AppFontSettings() {}
};

#endif // APPFONTSETTINGS_H