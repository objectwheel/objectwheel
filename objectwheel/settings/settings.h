#ifndef SETTINGS_H
#define SETTINGS_H

#include <QVariant>

class GroupSettings;
struct Settings
{
    friend class SettingsManager; // For setValue()

    explicit Settings(GroupSettings* groupSettings);
    virtual void read() = 0;
    virtual void write() = 0;
    virtual void reset() = 0;
    virtual const char* category() const = 0;
    GroupSettings* groupSettings() const;

    void begin() const;
    void end() const;
    template<typename ReturnType>
    ReturnType value(const char* setting, const QVariant& defaultValue);

protected:
    void setValue(const char* setting, const QVariant& value);

private:
    QString path(const char* setting) const;
    QVariant valueHelper(const char* setting, const QVariant& defaultValue);

private:
    GroupSettings* m_groupSettings;
};

template<typename T>
inline T Settings::value(const char* setting, const QVariant& defaultValue)
{
    return qvariant_cast<T>(valueHelper(setting, defaultValue));
}

#endif // SETTINGS_H