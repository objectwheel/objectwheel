#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QToolBar;
class FormsPane;
class LoadingBar;
class FlatButton;
class QDockWidget;
class ToolboxPane;
class InspectorPane;
class CentralWidget;
class PropertiesPane;

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        CentralWidget* centralWidget();

    private slots:
        void clear();
        void handleRunButtonClick();
        void handleStopButtonClick();
        void handleStopButtonDoubleClick();
        void handleBuildsButtonClick();
        void handleProjectsButtonClick();

    signals:
        void done();

    private:
        QToolBar* _toolBar;
        QDockWidget* _toolboxDockwidget;
        QDockWidget* _propertiesDockwidget;
        QDockWidget* _formsDockwidget;
        QDockWidget* _inspectorDockwidget;
        CentralWidget* _centralWidget;
        ToolboxPane* _toolboxPane;
        PropertiesPane* _propertiesPane;
        FormsPane* _formsPane;
        InspectorPane* _inspectorPage;
        LoadingBar* _loadingBar;
        FlatButton* _runButton;
        FlatButton* _stopButton;
        FlatButton* _buildsButton;
        FlatButton* _projectsButton;

};

#endif // MAINWINDOW_H
