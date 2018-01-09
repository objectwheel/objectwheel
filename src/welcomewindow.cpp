#include <welcomewindow.h>
#include <loginwidget.h>
#include <projectswidget.h>
#include <newprojectwidget.h>
#include <registrationwidget.h>
#include <view.h>
#include <fit.h>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);
    setWindowTitle(APP_NAME);

    _loginWidget = new LoginWidget;
    _registrationWidget = new RegistrationWidget;
    _projectsWidget = new ProjectsWidget;
    _newProjectWidget = new NewProjectWidget;

    connect(_loginWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_projectsWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_loginWidget, SIGNAL(done()), SLOT(showProjects()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(lazy()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(done()));

    _view = new View(this);
    _view->add(Login, _loginWidget);
    _view->add(Registration, _registrationWidget);
    _view->add(Projects, _projectsWidget);
    _view->add(NewProject, _newProjectWidget);
    _view->show(Login);
}

void WelcomeWindow::showLogin()
{
    _view->show(Login, View::LeftToRight);
}

void WelcomeWindow::showProjects()
{
//    _projectsWidget->refreshProjectList(); //FIXME
    _view->show(Projects, View::RightToLeft);
    emit lazy();
}

void WelcomeWindow::resizeEvent(QResizeEvent* event)
{
    _view->setGeometry(rect());
    QWidget::resizeEvent(event);
}