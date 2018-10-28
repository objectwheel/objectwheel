#include <windowmanager.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>
#include <preferenceswindow.h>
#include <toolboxsettingswindow.h>
#include <projectmanager.h>
#include <applicationcore.h>

#include <QStyle>
#include <QApplication>
#include <QScreen>

AboutWindow* WindowManager::s_aboutWindow = nullptr;
PreferencesWindow* WindowManager::s_preferencesWindow = nullptr;
ToolboxSettingsWindow* WindowManager::s_toolboxSettingsWindow = nullptr;
MainWindow* WindowManager::s_mainWindow = nullptr;
WelcomeWindow* WindowManager::s_welcomeWindow = nullptr;

WindowManager::WindowManager(QObject* parent) : QObject(parent)
{
    s_aboutWindow = new AboutWindow;
    s_preferencesWindow = new PreferencesWindow;
    // s_toolboxSettingsWindow = new ToolboxSettingsWindow;
    s_mainWindow = new MainWindow;
    s_welcomeWindow = new WelcomeWindow;

    QObject::connect(s_aboutWindow, &AboutWindow::done, s_aboutWindow, &AboutWindow::hide);
    QObject::connect(s_preferencesWindow, &PreferencesWindow::done,
                     s_preferencesWindow, &PreferencesWindow::hide);
//    QObject::connect(s_toolboxSettingsWindow, &ToolboxSettingsWindow::done,
//                     s_toolboxSettingsWindow, &ToolboxSettingsWindow::hide);
    QObject::connect(s_mainWindow, &MainWindow::done, s_mainWindow, &MainWindow::hide);
    QObject::connect(s_welcomeWindow, &WelcomeWindow::done, s_welcomeWindow, &WelcomeWindow::hide);
    QObject::connect(s_welcomeWindow, &WelcomeWindow::done, s_mainWindow, &MainWindow::show);

    s_aboutWindow->setGeometry(QStyle::alignedRect(
                                   Qt::LeftToRight, Qt::AlignCenter, s_aboutWindow->sizeHint(),
                                   qApp->primaryScreen()->availableGeometry()));
    s_preferencesWindow->setGeometry(QStyle::alignedRect(
                                         Qt::LeftToRight, Qt::AlignCenter, s_preferencesWindow->sizeHint(),
                                         qApp->primaryScreen()->availableGeometry()));
//    s_toolboxSettingsWindow->setGeometry(QStyle::alignedRect(
//                                             Qt::LeftToRight, Qt::AlignCenter, s_toolboxSettingsWindow->sizeHint(),
//                                             qApp->primaryScreen()->availableGeometry()));
    s_mainWindow->setGeometry(QStyle::alignedRect(
                                  Qt::LeftToRight, Qt::AlignCenter, s_mainWindow->sizeHint(),
                                  qApp->primaryScreen()->availableGeometry()));
    s_welcomeWindow->setGeometry(QStyle::alignedRect(
                                     Qt::LeftToRight, Qt::AlignCenter, s_welcomeWindow->sizeHint(),
                                     qApp->primaryScreen()->availableGeometry()));
}

WindowManager::~WindowManager()
{
    // FIXME
//    delete s_welcomeWindow;
    s_welcomeWindow->deleteLater();
//    delete s_mainWindow;
    s_mainWindow->deleteLater();

//    delete s_toolboxSettingsWindow;
    s_preferencesWindow->deleteLater();
    delete s_aboutWindow;
}

WelcomeWindow* WindowManager::welcomeWindow()
{
    return s_welcomeWindow;
}

MainWindow* WindowManager::mainWindow()
{
    return s_mainWindow;
}

ToolboxSettingsWindow* WindowManager::toolboxSettingsWindow()
{
    return s_toolboxSettingsWindow;
}

PreferencesWindow* WindowManager::preferencesWindow()
{
    return s_preferencesWindow;
}

AboutWindow* WindowManager::aboutWindow()
{
    return s_aboutWindow;
}