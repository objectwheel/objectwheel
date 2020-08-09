#include <windowmanager.h>
#include <welcomewindow.h>
#include <mainwindow.h>
#include <aboutwindow.h>
#include <preferenceswindow.h>
#include <projectmanager.h>
#include <applicationcore.h>
#include <subscriptionwindow.h>

#include <QStyle>
#include <QApplication>
#include <QScreen>

AboutWindow* WindowManager::s_aboutWindow = nullptr;
PreferencesWindow* WindowManager::s_preferencesWindow = nullptr;
MainWindow* WindowManager::s_mainWindow = nullptr;
WelcomeWindow* WindowManager::s_welcomeWindow = nullptr;
SubscriptionWindow* WindowManager::s_subscriptionWindow = nullptr;

WindowManager::WindowManager(QObject* parent) : QObject(parent)
{
    s_aboutWindow = new AboutWindow;
    s_preferencesWindow = new PreferencesWindow;
    s_mainWindow = new MainWindow;
    s_welcomeWindow = new WelcomeWindow;
    s_subscriptionWindow = new SubscriptionWindow;

    connect(s_aboutWindow, &AboutWindow::done,
            s_aboutWindow, &AboutWindow::hide);
    connect(s_preferencesWindow, &PreferencesWindow::done,
            s_preferencesWindow, &PreferencesWindow::hide);
    connect(s_mainWindow, &MainWindow::done, s_mainWindow,
            &MainWindow::hide);
    connect(s_welcomeWindow, &WelcomeWindow::done,
            s_welcomeWindow, &WelcomeWindow::hide);
    connect(s_welcomeWindow, &WelcomeWindow::done,
            s_mainWindow, &MainWindow::show);
    connect(s_welcomeWindow, &WelcomeWindow::done,
            s_mainWindow, &MainWindow::activateWindow);
    connect(s_welcomeWindow, &WelcomeWindow::subscriptionNeeded,
            s_welcomeWindow, &WelcomeWindow::hide);
    connect(s_welcomeWindow, &WelcomeWindow::subscriptionNeeded,
            s_subscriptionWindow, &SubscriptionWindow::show);
    connect(s_welcomeWindow, &WelcomeWindow::subscriptionNeeded,
            s_subscriptionWindow, &SubscriptionWindow::activateWindow);
    connect(s_welcomeWindow, &WelcomeWindow::subscriptionNeeded,
            s_subscriptionWindow, &SubscriptionWindow::reset);
    connect(s_subscriptionWindow, &SubscriptionWindow::done,
            s_subscriptionWindow, &SubscriptionWindow::hide);
    connect(s_subscriptionWindow, &SubscriptionWindow::done,
            s_welcomeWindow, &WelcomeWindow::show);
    connect(s_subscriptionWindow, &SubscriptionWindow::done,
            s_welcomeWindow, &WelcomeWindow::activateWindow);
}

WindowManager::~WindowManager()
{
    delete s_subscriptionWindow;
    s_subscriptionWindow = nullptr;
    delete s_welcomeWindow;
    s_welcomeWindow = nullptr;
    delete s_mainWindow;
    s_mainWindow = nullptr;
    delete s_preferencesWindow;
    s_preferencesWindow = nullptr;
    delete s_aboutWindow;
    s_aboutWindow = nullptr;
}

WelcomeWindow* WindowManager::welcomeWindow()
{
    return s_welcomeWindow;
}

SubscriptionWindow* WindowManager::subscriptionWindow()
{
    return s_subscriptionWindow;
}

MainWindow* WindowManager::mainWindow()
{
    return s_mainWindow;
}

PreferencesWindow* WindowManager::preferencesWindow()
{
    return s_preferencesWindow;
}

AboutWindow* WindowManager::aboutWindow()
{
    return s_aboutWindow;
}
