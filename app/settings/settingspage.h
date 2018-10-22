#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class QLabel;
class QTabWidget;
class SettingsWidget;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

    void reset();
    void apply();
    void activateCurrent();
    void setTitle(const QString& title);
    QString title() const;

    virtual QIcon icon() const = 0;
    virtual bool containsWord(const QString& word) const = 0;

    void addWidget(SettingsWidget* widget);
    QList<SettingsWidget*> widgets() const;

private:
    QTabWidget* m_tabWidget;
    QLabel* m_titleLabel;
};

Q_DECLARE_METATYPE(SettingsPage*)

#endif // SETTINGSPAGE_H