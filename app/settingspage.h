#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

class QLabel;
class QTabWidget;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

    virtual QString title() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool containsWord(const QString& word) const = 0;

    void addWidget(const QString& title, QWidget* widget);

protected:
    QSize sizeHint() const override;

private:
    QTabWidget* m_tabWidget;
    QLabel* m_titleLabel;
};

Q_DECLARE_METATYPE(SettingsPage*)

#endif // SETTINGSPAGE_H