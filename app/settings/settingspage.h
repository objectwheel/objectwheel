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

//    virtual void apply() charge
//    discharge
//    load
//    unload
    void reset();
    void apply();
    void activateCurrent();
    virtual QIcon icon() const = 0;
    virtual QString title() const = 0;
    virtual bool containsWord(const QString& word) const = 0;

    void addWidget(SettingsWidget* widget);
    QList<SettingsWidget*> widgets() const;

protected:
    QSize sizeHint() const override;

private:
    QTabWidget* m_tabWidget;
    QLabel* m_titleLabel;
};

Q_DECLARE_METATYPE(SettingsPage*)

#endif // SETTINGSPAGE_H