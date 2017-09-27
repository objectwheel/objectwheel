#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QQuickWidget>
#include <QQuickItem>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

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
class CoverMenu;

class MainWindow : public QWidget
{
        Q_OBJECT

    private:
        CoverMenu* _rightMenu;
        CoverMenu* _leftMenu;
        QWidget* _centralWidget;
        ProjectsScreen* _projectsScreen;
        LoginScreen* _loginScreen;
        QVBoxLayout* verticalLayout;
        TitleBar* titleBar;
        QWidget* toolboxWidget;
        QVBoxLayout* toolboxVLay;
        QWidget* toolboxAdderAreaWidget;
        QVBoxLayout* toolboxAdderAreaVLay;
        QHBoxLayout* toolboxAdderAreaButtonSideHLay;
        ListWidget* toolboxList;
        ListWidget* sceneList;
        FlatButton* toolboxAddButton;
        FlatButton* toolboxEditButton;
        FlatButton* toolboxRemoveButton;
        FlatButton* toolboxResetButton;
        FlatButton* toolboxExportButton;
        FlatButton* toolboxImportButton;
        QVBoxLayout* toolboxAdderAreaEditingLayout;
        LineEdit* toolboxUrlBox;
        LineEdit* toolBoxNameBox;
        PropertiesWidget* propertiesWidget;
        FormsWidget* formsWidget;
        About* aboutWidget;
        BuildsScreen* buildsScreen;
        FlatButton* aboutButton;
        QWidget* settleWidget;
        DesignManager* designManager;

    public:
        explicit MainWindow(QWidget *parent = 0);
        void SetupGui();
        void SetupManagers();
        void resizeEvent(QResizeEvent *event);
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

    signals:
        void resized();
};

#endif // MAINWINDOW_H
