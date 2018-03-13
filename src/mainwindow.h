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

    signals:
        void done();

    private:
        CentralWidget* m_centralWidget;
        OutputPane* m_outputPane;
        RunPane* m_runPane;
        FormsPane* m_formsPane;
        ToolboxPane* m_toolboxPane;
        InspectorPane* m_inspectorPane;
        PropertiesPane* m_propertiesPane;
        PageSwitcherPane* m_pageSwitcherPane;
};

#endif // MAINWINDOW_H
