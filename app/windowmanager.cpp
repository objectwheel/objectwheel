#include <windowmanager.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>
#include <preferenceswindow.h>
#include <toolboxsettingswindow.h>
#include <savebackend.h>
#include <dpr.h>
#include <projectbackend.h>
#include <backendmanager.h>
#include <QStyle>

AboutWindow* WindowManager::s_aboutWindow = nullptr;
PreferencesWindow* WindowManager::s_preferencesWindow = nullptr;
ToolboxSettingsWindow* WindowManager::s_toolboxSettingsWindow = nullptr;
MainWindow* WindowManager::s_mainWindow = nullptr;
WelcomeWindow* WindowManager::s_welcomeWindow = nullptr;

WindowManager::WindowManager()
{
    s_aboutWindow = new AboutWindow;
    s_preferencesWindow = new PreferencesWindow;
    s_toolboxSettingsWindow = new ToolboxSettingsWindow;
    s_mainWindow = new MainWindow;
    s_welcomeWindow = new WelcomeWindow;

    QObject::connect(s_aboutWindow, &AboutWindow::done, s_aboutWindow, &AboutWindow::hide);
    QObject::connect(s_preferencesWindow, &PreferencesWindow::done,
                     s_preferencesWindow, &PreferencesWindow::hide);
    QObject::connect(s_toolboxSettingsWindow, &ToolboxSettingsWindow::done,
                     s_toolboxSettingsWindow, &ToolboxSettingsWindow::hide);
    QObject::connect(s_mainWindow, &MainWindow::done, s_mainWindow, &MainWindow::hide);
    QObject::connect(s_welcomeWindow, &WelcomeWindow::done, s_welcomeWindow, &WelcomeWindow::hide);
    QObject::connect(s_welcomeWindow, &WelcomeWindow::done, s_mainWindow, &MainWindow::show);

    QObject::connect(ProjectBackend::instance(), &ProjectBackend::started,
            WindowManager::mainWindow(), &MainWindow::reset);
    QObject::connect(ProjectBackend::instance(), &ProjectBackend::started,
            BackendManager::instance(), &BackendManager::onProjectStart);

    s_aboutWindow->setGeometry(QStyle::alignedRect(
                                   Qt::LeftToRight, Qt::AlignCenter, s_aboutWindow->sizeHint(),
                                   QGuiApplication::primaryScreen()->availableGeometry()));
    s_preferencesWindow->setGeometry(QStyle::alignedRect(
                                         Qt::LeftToRight, Qt::AlignCenter, s_preferencesWindow->sizeHint(),
                                         QGuiApplication::primaryScreen()->availableGeometry()));
    s_toolboxSettingsWindow->setGeometry(QStyle::alignedRect(
                                             Qt::LeftToRight, Qt::AlignCenter, s_toolboxSettingsWindow->sizeHint(),
                                             QGuiApplication::primaryScreen()->availableGeometry()));
    s_mainWindow->setGeometry(QStyle::alignedRect(
                                  Qt::LeftToRight, Qt::AlignCenter, s_mainWindow->sizeHint(),
                                  QGuiApplication::primaryScreen()->availableGeometry()));
    s_welcomeWindow->setGeometry(QStyle::alignedRect(
                                     Qt::LeftToRight, Qt::AlignCenter, s_welcomeWindow->sizeHint(),
                                     QGuiApplication::primaryScreen()->availableGeometry()));
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

void WindowManager::init()
{
    static WindowManager instance;
    Q_UNUSED(instance);
}
