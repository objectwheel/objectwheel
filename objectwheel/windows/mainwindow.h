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
class ControlsPane;
class ControlsController;
class FormsPane;
class FormsController;
class AssetsPane;
class AssetsController;
class CentralWidget;
class DockBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit MainWindow(QWidget* parent = nullptr);

    CentralWidget* centralWidget() const;
    AssetsPane* assetsPane() const;
    ToolboxPane* toolboxPane() const;
    PropertiesPane* propertiesPane() const;
    ControlsPane* controlsPane() const;
    QSize sizeHint() const override;

    void setDockWidgetAreasVisible(Qt::DockWidgetAreas areas, bool visible);

public slots:
    void charge();
    void discharge();

private slots:
    void onModeChange(ModeManager::Mode mode);
    void onScreenChange(QScreen* screen);
    void onDockWidgetLocationChange(Qt::DockWidgetArea area);
    void onPinBarDockWidgetCloseButtonClick(QDockWidget* dockWidget);
    void onDockBarDockWidgetButtonClick(QDockWidget* dockWidget, bool checked);
    void onRemoveSizeRestrictionsOnDockWidgetsTimerTimeout();

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void resetWidget();
    void readSettings();
    void writeSettings() const;

signals:
    void done();

private:
    QByteArray m_initialInterfaceState;
    CentralWidget* m_centralWidget;
    RunPane* m_runPane;
    RunController* m_runController;
    ModeSelectorPane* m_modeSelectorPane;
    ModeSelectorController* m_modeSelectorController;
    ToolboxPane* m_toolboxPane;
    ToolboxController* m_toolboxController;
    PropertiesPane* m_propertiesPane;
    PropertiesController* m_propertiesController;
    ControlsPane* m_controlsPane;
    ControlsController* m_controlsController;
    FormsPane* m_formsPane;
    FormsController* m_formsController;
    AssetsPane* m_assetsPane;
    AssetsController* m_assetsController;
    QDockWidget* m_controlsDockWidget;
    QDockWidget* m_propertiesDockWidget;
    QDockWidget* m_assetsDockWidget;
    QDockWidget* m_toolboxDockWidget;
    QDockWidget* m_formsDockWidget;
    DockBar* m_leftDockBar;
    DockBar* m_rightDockBar;
    QTimer* m_removeSizeRestrictionsOnDockWidgetsTimer;
    const bool m_shownForTheFirstTime;
    bool m_assetsDockWidgetVisible;
    bool m_propertiesDockWidgetVisible;
    bool m_formsDockWidgetVisible;
    bool m_toolboxDockWidgetVisible;
    bool m_controlsDockWidgetVisible;
};

#endif // MAINWINDOW_H
