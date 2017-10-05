#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include <flatbutton.h>
#include <toolboxtree.h>
#include <propertieswidget.h>
#include <about.h>
#include <formswidget.h>
#include <fit.h>
#include <filemanager.h>
#include <mainwindow.h>
#include <projectsscreen.h>
#include <loginscreen.h>
#include <buildsScreen.h>
#include <designmanager.h>
#include <toolbox.h>

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        void setupGui();
        void setupManagers();
        void clearStudio();

    public slots:
        void handleIndicatorChanges();
        void cleanupObjectwheel();

    private:
        ProjectsScreen _projectsScreen;
        LoginScreen _loginScreen;
        BuildsScreen _buildsScreen;
        QWidget _centralWidget;
        QToolBar _titleBar;
        DesignManager _designManager;
        QDockWidget _toolboxDockwidget;
        QDockWidget _propertiesDockwidget;
        QDockWidget _formsDockwidget;
        ToolBox _toolbox;
        PropertiesWidget _propertiesWidget;
        FormsWidget _formsWidget;
        About _aboutWidget;
};

#endif // MAINWINDOW_H
