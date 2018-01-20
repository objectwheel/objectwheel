#include <fit.h>
#include <view.h>
#include <welcomewindow.h>
#include <loginwidget.h>
#include <projectswidget.h>
#include <newprojectwidget.h>
#include <registrationwidget.h>
#include <verificationwidget.h>

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
    _verificationWidget = new VerificationWidget;

    connect(_projectsWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_loginWidget, SIGNAL(done()), SLOT(showProjects()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(lazy()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(done()));
    connect(_registrationWidget, SIGNAL(done(QString)),
      _verificationWidget, SLOT(setEmail(QString)));

    connect(_loginWidget, &LoginWidget::signup, [=] {
        _view->show(Registration, View::LeftToRight);
    });

    connect(_registrationWidget, &RegistrationWidget::cancel, [=] {
        _view->show(Login, View::RightToLeft);
    });

    connect(_registrationWidget, &RegistrationWidget::done, [=] {
        _view->show(Verification, View::RightToLeft);
    });

    connect(_verificationWidget, &VerificationWidget::cancel, [=] {
        _view->show(Login, View::LeftToRight);
    });

    connect(_verificationWidget, &VerificationWidget::done, [=] {
        _view->show(Login, View::LeftToRight);
    });

    _view = new View(this);
    _view->add(Login, _loginWidget);
    _view->add(Registration, _registrationWidget);
    _view->add(Verification, _verificationWidget);
    _view->add(Projects, _projectsWidget);
    _view->add(NewProject, _newProjectWidget);
    _view->show(Registration);
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