#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <settingswidget.h>

class QLabel;
class QTabWidget;

class SettingsPage : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsPage)

public:
    explicit SettingsPage(QWidget* parent = nullptr);
    void revert();
    void reset();
    void apply();
    void activateCurrent();
    void setTitle(const QString& title);
    QString title() const;

    virtual QIcon icon() const = 0;
    virtual bool containsWord(const QString& word) const;

    void addWidget(SettingsWidget* widget);
    QList<SettingsWidget*> widgets() const;

private:
    QTabWidget* m_tabWidget;
    QLabel* m_titleLabel;
};

#endif // SETTINGSPAGE_H