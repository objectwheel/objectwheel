#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

#include <QWidget>
#include <QHash>

class QLabel;
class QGridLayout;
class QListWidget;
class FocuslessLineEdit;
class QDialogButtonBox;
class SettingsPage;

class PreferencesWindow : public QWidget
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit PreferencesWindow(QWidget *parent = nullptr);

private slots:
    void search(const QString& text);

protected:
     QSize sizeHint() const override;

private:
   void addPage(SettingsPage* page);

signals:
    void done();

private:
    QGridLayout* m_layout;
    QListWidget* m_listWidget;
    FocuslessLineEdit* m_searchLineEdit;
    QDialogButtonBox* m_dialogButtonBox;
    QHash<QString, SettingsPage*> m_settingsPages;
};

#endif // PREFERENCESWINDOW_H