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
#include <buildsdialog.h>
#include <designmanager.h>
#include <toolbox.h>
#include <progresswidget.h>
#include <inspectorwidget.h>
#include <preferencesdialog.h>

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = 0);
        static MainWindow* instance();
        ProgressWidget* progressWidget();
        InspectorWidget* inspectorWidget();
        BuildsDialog* buildsDialog();

    public slots:
        void setupGui();
        void setupManagers();
        void clearStudio();
        void handleIndicatorChanges();
        void cleanupObjectwheel();
        void showDockWidgets();
        void hideDockWidgets();
        void newFile();
        void open();
        void save();
        void print();
        void undo();
        void redo();
        void cut();
        void copy();
        void paste();
        void bold();
        void italic();
        void leftAlign();
        void rightAlign();
        void justify();
        void center();
        void setLineSpacing();
        void setParagraphSpacing();
        void preferences();
        void about();

    protected:
        void closeEvent(QCloseEvent* event) override;

    signals:
        void done();
        void lazy();
        void busy(const QString& text);
        void quitting() const;

    private:
        void createActions();
        void createMenus();

    private:
        static MainWindow* _instance;
        PreferencesDialog _preferencesWidget;
        BuildsDialog _buildsDialog;
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
        QMenu* fileMenu;
        QMenu* editMenu;
        QMenu* formatMenu;
        QMenu* helpMenu;
        QMenu* optionsMenu;
        QActionGroup* alignmentGroup;
        QAction* newAct;
        QAction* openAct;
        QAction* saveAct;
        QAction* printAct;
        QAction* exitAct;
        QAction* undoAct;
        QAction* redoAct;
        QAction* cutAct;
        QAction* copyAct;
        QAction* pasteAct;
        QAction* boldAct;
        QAction* italicAct;
        QAction* leftAlignAct;
        QAction* rightAlignAct;
        QAction* justifyAct;
        QAction* centerAct;
        QAction* setLineSpacingAct;
        QAction* setParagraphSpacingAct;
        QAction* preferencesAct;
        QAction* aboutAct;
        QAction* aboutQtAct;
};

#endif // MAINWINDOW_H
