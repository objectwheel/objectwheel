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
        explicit MainWindow(QWidget* parent = 0);

    public slots:
        void clear();

    signals:
        void done();

    private:
        OutputPane* m_outputPane;
        RunPane* m_runPane;
        PageSwitcherPane* m_pageSwitcherPane;
        ToolboxPane* m_toolboxPane;
        PropertiesPane* m_propertiesPane;
        FormsPane* m_formsPane;
        InspectorPane* m_inspectorPage;
        CentralWidget* m_centralWidget;
};

#endif // MAINWINDOW_H
