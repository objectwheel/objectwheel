#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <modemanager.h>
#include <QMainWindow>

class RunPane;
class RunController;
class ModeSelectorPane;
class ModeSelectorController;
class ToolboxPane;
class ToolboxController;
class PropertiesPane;
class PropertiesController;
class NavigatorPane;
class NavigatorController;
class FormsPane;
class CentralWidget;
class AssetsPane;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit MainWindow(QWidget* parent = nullptr);

    CentralWidget* centralWidget() const;
    NavigatorPane* navigatorPane() const;
    PropertiesPane* propertiesPane() const;
    AssetsPane* assetsPane() const;
    ToolboxPane* toolboxPane() const;

public slots:
    void charge();
    void discharge();
    void showLeftPanes(bool);
    void showRightPanes(bool);

private slots:
    void hideDocks();
    void showDocks();
    void restoreDocks();
    void onModeChange(ModeManager::Mode mode);
    void onScreenChange(QScreen* screen);

protected:
    QSize sizeHint() const override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void resetSettings();
    void readSettings();
    void writeSettings();

signals:
    void done();

private:
    QByteArray m_resetState;
    CentralWidget* m_centralWidget;
    RunPane* m_runPane;
    RunController* m_runController;
    ModeSelectorPane* m_modeSelectorPane;
    ModeSelectorController* m_modeSelectorController;
    ToolboxPane* m_toolboxPane;
    ToolboxController* m_toolboxController;
    PropertiesPane* m_propertiesPane;
    PropertiesController* m_propertiesController;
    NavigatorPane* m_navigatorPane;
    NavigatorController* m_navigatorController;
    FormsPane* m_formsPane;
    AssetsPane* m_assetsPane;
    QDockWidget* m_navigatorDockWidget;
    QDockWidget* m_propertiesDockWidget;
    QDockWidget* m_assetsDockWidget;
    QDockWidget* m_toolboxDockWidget;
    QDockWidget* m_formsDockWidget;
    bool m_assetsDockWidgetVisible;
    bool m_propertiesDockWidgetVisible;
    bool m_formsDockWidgetVisible;
    bool m_toolboxDockWidgetVisible;
    bool m_navigatorDockWidgetVisible;
};

#endif // MAINWINDOW_H
