#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include <centralwidget.h>
#include <flatbutton.h>
#include <toolboxtree.h>
#include <propertieswidget.h>
#include <aboutwidget.h>
#include <formswidget.h>
#include <fit.h>
#include <filemanager.h>
#include <mainwindow.h>
#include <projectsscreen.h>
#include <loginscreen.h>
#include <buildsscreen.h>
#include <designmanager.h>
#include <toolbox.h>
#include <progresswidget.h>
#include <inspectorwidget.h>

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        static MainWindow* instance();
        CentralWidget* centralWidget();
        ProgressWidget* progressWidget();
        InspectorWidget* inspectorWidget();
        BuildsScreen* buildsScreen();

        void setupGui();
        void setupManagers();
        void clearStudio();


    public slots:
        void handleIndicatorChanges();
        void cleanupObjectwheel();
        void showDockWidgets();
        void hideDockWidgets();

    protected:
        void closeEvent(QCloseEvent *event) override;

    signals:
        void quitting() const;

    private:
        static MainWindow* _instance;
        CentralWidget _centralWidget;
        ProgressWidget _progressWidget;
        ProjectsScreen _projectsScreen;
        LoginScreen _loginScreen;
        BuildsScreen _buildsScreen;
        QFrame _settleWidget;
        QToolBar _titleBar;
        QDockWidget _toolboxDockwidget;
        QDockWidget _propertiesDockwidget;
        QDockWidget _formsDockwidget;
        QDockWidget _inspectorDockwidget;
        DesignManager _designManager;
        ToolBox _toolbox;
        PropertiesWidget _propertiesWidget;
        FormsWidget _formsWidget;
        InspectorWidget _inspectorWidget;
        AboutWidget _aboutWidget;
};

#endif // MAINWINDOW_H
