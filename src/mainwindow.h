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
class DesignerWidget;
class PropertiesPane;
class ExecutiveWidget;

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        DesignerWidget* designerWidget();

    private slots:
        void cleanupObjectwheel();
        void handleRunButtonClick();
        void handleStopButtonClick();
        void handleBuildsButtonClick();

    signals:
        void done();

    private:
        QToolBar* _titleBar;
        QDockWidget* _toolboxDockwidget;
        QDockWidget* _propertiesDockwidget;
        QDockWidget* _formsDockwidget;
        QDockWidget* _inspectorDockwidget;
        DesignerWidget* _designerWidget;
        ToolboxPane* _toolboxPane;
        PropertiesPane* _propertiesPane;
        FormsPane* _formsPane;
        InspectorPane* _inspectorPage;
        LoadingBar* _loadingBar;
        FlatButton* _runButton;
        FlatButton* _stopButton;
        FlatButton* _buildsButton;
        ExecutiveWidget* _executiveWidget;

};

#endif // MAINWINDOW_H
