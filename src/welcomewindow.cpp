#include <welcomewindow.h>
#include <projectswidget.h>
#include <loginwidget.h>
#include <view.h>
#include <fit.h>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle(APP_NAME);

    _loginWidget = new LoginWidget;
    _projectsWidget = new ProjectsWidget;

    connect(_loginWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_projectsWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_loginWidget, SIGNAL(done()), SLOT(showProjects()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(lazy()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(done()));

    _view = new View(this);
    _view->add(Projects, _projectsWidget);
    _view->add(Login, _loginWidget);
    _view->show(Login);
}

void WelcomeWindow::showLogin()
{
    _view->show(Login, View::LeftToRight);
}

void WelcomeWindow::resizeEvent(QResizeEvent* event)
{
    _view->setGeometry(rect());
    QWidget::resizeEvent(event);
}

void WelcomeWindow::showProjects()
{
    _projectsWidget->refreshProjectList();
    _view->show(Projects, View::RightToLeft);
    emit lazy();
}
