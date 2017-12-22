#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include <windowmanager.h>
#include <flatbutton.h>
#include <toolboxtree.h>
#include <propertieswidget.h>
#include <aboutwindow.h>
#include <formswidget.h>
#include <fit.h>
#include <filemanager.h>
#include <mainwindow.h>
#include <projectswidget.h>
#include <designmanager.h>
#include <toolbox.h>
#include <progresswidget.h>
#include <inspectorwidget.h>
#include <preferenceswindow.h>

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        static MainWindow* instance();
        InspectorWidget* inspectorWidget();

    public slots:
        void setupGui();
        void setupManagers();
        void clearStudio();
        void handleIndicatorChanges();
        void cleanupObjectwheel();
        void showDockWidgets();
        void hideDockWidgets();

    protected:
        void closeEvent(QCloseEvent* event) override;

    signals:
        void done();
        void quitting() const;

    private:
        static MainWindow* _instance;
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
};

#endif // MAINWINDOW_H
