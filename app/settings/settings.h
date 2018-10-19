#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings
{
public:
    Settings(const QString& group);

    virtual ~Settings() {}
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void reset() = 0;

    const QString& group() const;

private:
    QString m_group;
};

#endif // SETTINGS_H