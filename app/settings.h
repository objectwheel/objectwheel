#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Settings)

    Settings();
    ~Settings() {}

public:
    static Settings* instance();


private:

};

#endif // SETTINGS_H