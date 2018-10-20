#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject* parent = nullptr);
    explicit Settings(const QString& group, QObject* parent = nullptr);

    virtual void read() = 0;
    virtual void write() = 0;
    virtual void reset() = 0;

    const QString& group() const;

signals:
    void changed();

private:
    QString m_group;
};

#endif // SETTINGS_H