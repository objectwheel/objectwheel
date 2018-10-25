#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QScrollArea>

class QVBoxLayout;

class SettingsWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget* parent = nullptr);

    virtual void apply() = 0;
    virtual void reset() = 0;
    virtual QIcon icon() const = 0;
    virtual QString title() const = 0;

    void activate(bool activate = true);
    bool isActivated() const;

    QWidget* contentWidget() const;
    QVBoxLayout* contentLayout() const;

private:
    bool m_activated;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
};

#endif // SETTINGSWIDGET_H