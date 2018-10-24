#ifndef APPFONTSETTINGS_H
#define APPFONTSETTINGS_H

#include <QFont>

class DefaultFont final
{
    Q_DISABLE_COPY(DefaultFont)

public:
    DefaultFont() = delete;
    static void load();
    static QFont font();
};

#endif // APPFONTSETTINGS_H