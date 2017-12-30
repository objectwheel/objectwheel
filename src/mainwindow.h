#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include <windowmanager.h>
#include <flatbutton.h>
#include <toolboxtree.h>
#include <propertiespane.h>
#include <aboutwindow.h>
#include <formspane.h>
#include <fit.h>
#include <filemanager.h>
#include <mainwindow.h>
#include <projectswidget.h>
#include <designerwidget.h>
#include <toolboxpane.h>
#include <progresswidget.h>
#include <inspectorpane.h>
#include <preferenceswindow.h>

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        DesignerWidget* designerWidget();

    private slots:
        void cleanupObjectwheel();

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
};

#endif // MAINWINDOW_H
