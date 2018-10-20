#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QAbstractScrollArea>

class SettingsWidget : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);

    void activate(bool activate = true);
    bool isActivated() const;

    virtual void apply() = 0;
    virtual void reset() = 0;
    virtual QString title() const = 0;

private:
    bool m_activated;
};

#endif // SETTINGSWIDGET_H