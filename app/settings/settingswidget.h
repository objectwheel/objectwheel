#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QScrollArea>

class QBoxLayout;
class SettingsWidget : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsWidget)

public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    virtual void apply() = 0;
    virtual void reset() = 0;
    virtual QIcon icon() const = 0;
    virtual QString title() const = 0;
    virtual bool containsWord(const QString& word) const = 0;

    void activate(bool activate = true);
    bool isActivated() const;

    QWidget* contentWidget() const;
    QBoxLayout* contentLayout() const;

private:
    bool m_activated;
};

#endif // SETTINGSWIDGET_H