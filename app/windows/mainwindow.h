#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <modemanager.h>
#include <QMainWindow>

class RunPane;
class RunController;
class ModeSelectorPane;
class ModeSelectorController;
class FormsPane;
class ToolboxPane;
class InspectorPane;
class CentralWidget;
class PropertiesPane;
class AssetsPane;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit MainWindow(QWidget* parent = nullptr);

    CentralWidget* centralWidget() const;
    InspectorPane* inspectorPane() const;
    PropertiesPane* propertiesPane() const;
    AssetsPane* assetsPane() const;

public slots:
    void discharge();
    void showLeftPanes(bool);
    void showRightPanes(bool);
    void setDockWidgetTitleBarsHidden(bool);

private slots:
    void hideDocks();
    void showDocks();
    void restoreDocks();
    void onModeChange(ModeManager::Mode mode);

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
    CentralWidget* m_centralWidget;
    RunPane* m_runPane;
    RunController* m_runController;
    ModeSelectorPane* m_modeSelectorPane;
    ModeSelectorController* m_modeSelectorController;
    FormsPane* m_formsPane;
    ToolboxPane* m_toolboxPane;
    InspectorPane* m_inspectorPane;
    PropertiesPane* m_propertiesPane;
    AssetsPane* m_assetsPane;
};

#endif // MAINWINDOW_H
