#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include <flatbutton.h>
#include <listwidget.h>
#include <propertieswidget.h>
#include <titlebar.h>
#include <about.h>
#include <formswidget.h>
#include <fit.h>
#include <lineedit.h>
#include <filemanager.h>
#include <mainwindow.h>
#include <projectsscreen.h>
#include <loginscreen.h>
#include <buildsscreen.h>
#include <designmanager.h>

class MainWindowPrivate;

class MainWindow : public QMainWindow
{
        Q_OBJECT

    private:
        ProjectsScreen _projectsScreen;
        LoginScreen _loginScreen;
        BuildsScreen buildsScreen;

        QWidget _centralWidget;
        QWidget settleWidget;
        QVBoxLayout verticalLayout;
        TitleBar titleBar;
        DesignManager designManager;

        QDockWidget _toolboxDockwidget;
        QDockWidget _propertiesDockwidget;
        QDockWidget _formsDockwidget;

        QWidget toolboxWidget;
        PropertiesWidget propertiesWidget;
        FormsWidget formsWidget;

        QVBoxLayout toolboxVLay;
        ListWidget toolboxList;
        QWidget toolboxAdderAreaWidget;
        QVBoxLayout toolboxAdderAreaVLay;
        QHBoxLayout toolboxAdderAreaButtonSideHLay;
        FlatButton toolboxAddButton;
        FlatButton toolboxEditButton;
        FlatButton toolboxRemoveButton;
        FlatButton toolboxResetButton;
        FlatButton toolboxExportButton;
        FlatButton toolboxImportButton;
        QVBoxLayout toolboxAdderAreaEditingLayout;
        LineEdit toolboxUrlBox;
        LineEdit toolBoxNameBox;

        About aboutWidget;

    public:
        explicit MainWindow(QWidget *parent = 0);
        void setupGui();
        void setupManagers();
        void clearStudio();
        void showAdderArea();
        void hideAdderArea();

    public slots:
        void on_secureExitButton_clicked();
        void handleToolboxUrlboxChanges(const QString& text);
        void handleToolboxNameboxChanges(QString name);
        void toolboxEditButtonToggled(bool);
        void toolboxRemoveButtonClicked();
        void toolboxAddButtonClicked();
        void toolboxImportButtonClicked();
        void toolboxExportButtonClicked();
        void handleImports(const QStringList& fileNames);
        void handleIndicatorChanges();
        void cleanupObjectwheel();
};

#endif // MAINWINDOW_H
