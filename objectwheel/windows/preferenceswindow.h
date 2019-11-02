#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

#include <QWidget>
#include <QHash>

class QLabel;
class QGridLayout;
class QListWidget;
class LineEdit;
class QDialogButtonBox;
class SettingsPage;
class SettingsWidget;

class PreferencesWindow : public QWidget
{
    Q_OBJECT

    friend class WindowManager;

public:
    enum Widget {
        ToolboxSettingsWidget,
        SceneSettingsWidget,
        InterfaceSettingsWidget,
        FontColorsSettingsWidget,
        BehaviorSettingsWidget
    };

public:
    explicit PreferencesWindow(QWidget* parent = nullptr);

    void setCurrentWidget(Widget widget);

    SettingsPage* page(Widget widget) const;
    SettingsWidget* widget(Widget widget) const;

private slots:
    void apply();
    void revert();
    void reset();
    void activateCurrent();
    void search(const QString& word);

protected:
    QSize sizeHint() const override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void resetSettings();
    void readSettings();
    void writeSettings();

private:
     void addPage(SettingsPage* page);
     void setCurrentPage(SettingsPage* page, SettingsPage* previous);

signals:
    void done();

private:
    QGridLayout* m_layout;
    QListWidget* m_listWidget;
    LineEdit* m_searchLineEdit;
    QDialogButtonBox* m_dialogButtonBox;
};

#endif // PREFERENCESWINDOW_H