#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class RunPane;
class FormsPane;
class OutputPane;
class ToolboxPane;
class InspectorPane;
class CentralWidget;
class PropertiesPane;
class PageSwitcherPane;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    void reset();

private slots:
    void hideDocks();
    void showDocks();

signals:
    void done();

private:
    CentralWidget* m_centralWidget;
    RunPane* m_runPane;
    FormsPane* m_formsPane;
    ToolboxPane* m_toolboxPane;
    InspectorPane* m_inspectorPane;
    PropertiesPane* m_propertiesPane;
    PageSwitcherPane* m_pageSwitcherPane;
};

#endif // MAINWINDOW_H
