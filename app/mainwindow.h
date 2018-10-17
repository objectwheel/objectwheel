#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class RunPane;
class FormsPane;
class ToolboxPane;
class InspectorPane;
class CentralWidget;
class PropertiesPane;
class GlobalResourcesPane;
class PageSwitcherPane;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class WindowManager;

public:
    explicit MainWindow(QWidget* parent = nullptr);

    CentralWidget* centralWidget() const;
    InspectorPane* inspectorPane() const;
    PropertiesPane* propertiesPane() const;
    GlobalResourcesPane* globalResourcesPane() const;

public slots:
    void sweep();
    void showLeftPanes(bool);
    void showRightPanes(bool);
    void setDockWidgetTitleBarsHidden(bool);

private slots:
    void hideDocks();
    void showDocks();
    void restoreDocks();

signals:
    void done();

private:
    QSize sizeHint() const override;

private:
    CentralWidget* m_centralWidget;
    RunPane* m_runPane;
    FormsPane* m_formsPane;
    ToolboxPane* m_toolboxPane;
    InspectorPane* m_inspectorPane;
    PropertiesPane* m_propertiesPane;
    GlobalResourcesPane* m_globalResourcesPane;
    PageSwitcherPane* m_pageSwitcherPane;
};

#endif // MAINWINDOW_H
