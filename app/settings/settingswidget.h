#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QAbstractScrollArea>

class SettingsWidget : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);

    inline void activate() { m_activated = true; }
    inline bool isActivated() const { return m_activated; }

    virtual QString title() const = 0;
    virtual void clean() = 0;
    virtual void apply() = 0;

public:
    bool m_activated;
};

#endif // SETTINGSWIDGET_H