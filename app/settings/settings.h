#ifndef SETTINGS_H
#define SETTINGS_H

#include <QVariant>

class GroupSettings;

struct Settings
{
    Settings(GroupSettings* groupSettings);
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void reset() = 0;
    virtual const char* category() const = 0;
    GroupSettings* groupSettings() const;

protected:
    void begin() const;
    void end() const;
    void setValue(const char* setting, const QVariant& value);
    QVariant value(const char* setting, const QVariant& defaultValue);

private:
    QString path(const char* setting) const;

private:
    GroupSettings* m_groupSettings;
};

#endif // SETTINGS_H