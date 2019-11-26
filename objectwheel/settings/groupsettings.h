#ifndef GROUPSETTINGS_H
#define GROUPSETTINGS_H

#include <QObject>

class GroupSettings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GroupSettings)

public:
    virtual const char* group() const = 0;

protected:
    explicit GroupSettings(QObject* parent = nullptr);
};

#endif // GROUPSETTINGS_H