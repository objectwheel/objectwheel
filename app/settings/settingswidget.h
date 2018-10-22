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

    QString title() const;
    void setTitle(const QString& title);

    virtual void apply() = 0;
    virtual void reset() = 0;

private:
    bool m_activated;
    QString m_title;
};

#endif // SETTINGSWIDGET_H