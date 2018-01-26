#include <fit.h>
#include <view.h>
#include <welcomewindow.h>
#include <loginwidget.h>
#include <robotwidget.h>
#include <projectswidget.h>
#include <newprojectwidget.h>
#include <registrationwidget.h>
#include <verificationwidget.h>
#include <registrationsucceedwidget.h>
#include <forgetwidget.h>
#include <windowmanager.h>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#e0e4e7");
    setPalette(p);
    setWindowTitle(APP_NAME);

    _layout = new QVBoxLayout(this);
    _view = new View;
    _loginWidget = new LoginWidget;
    _robotWidget = new RobotWidget;
    _registrationWidget = new RegistrationWidget;
    _projectsWidget = new ProjectsWidget;
    _newProjectWidget = new NewProjectWidget;
    _verificationWidget = new VerificationWidget;
    _registrationSucceedWidget = new RegistrationSucceedWidget;
    _forgetWidget = new ForgetWidget;

    _layout->setSpacing(0);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSizeConstraint(QLayout::SetMaximumSize);
    _layout->addWidget(_view);

    _view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _view->add(Login, _loginWidget);
    _view->add(Robot, _robotWidget);
    _view->add(Registration, _registrationWidget);
    _view->add(Verification, _verificationWidget);
    _view->add(Projects, _projectsWidget);
    _view->add(NewProject, _newProjectWidget);
    _view->add(RegistrationSucceed, _registrationSucceedWidget);
    _view->add(Forget, _forgetWidget);
    _view->show(Registration);

//    connect(_projectsWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
//    connect(_loginWidget, SIGNAL(done()), SLOT(showProjects()));
//    connect(_projectsWidget, SIGNAL(done()), SIGNAL(lazy()));
//    connect(_projectsWidget, SIGNAL(done()), SIGNAL(done()));

    connect(_forgetWidget, &ForgetWidget::back, [=] {
        _view->show(Login);
    });

    connect(_loginWidget, &LoginWidget::signup, [=] {
        _robotWidget->load();
        _view->show(Robot);
    });

    connect(_loginWidget, &LoginWidget::forget, [=] {
        _view->show(Forget);
    });

    connect(_loginWidget, &LoginWidget::about, [=] {
        WindowManager::instance()->show(WindowManager::About);
    });

    connect(_robotWidget, SIGNAL(done(QString)),
      _registrationWidget, SLOT(updateResponse(QString)));

    connect(_robotWidget, &RobotWidget::back, [=] {
        _view->show(Login);
    });

    connect(_robotWidget, &RobotWidget::done, [=] {
        _view->show(Registration);
    });

    connect(_registrationWidget, &RegistrationWidget::back, [=] {
        _view->show(Login);
    });

    connect(_registrationWidget, SIGNAL(done(QString)),
      _verificationWidget, SLOT(setEmail(QString)));

    connect(_registrationWidget, &RegistrationWidget::done, [=] {
        _view->show(Verification);
        _robotWidget->reset();
    });

    connect(_verificationWidget, &VerificationWidget::cancel, [=] {
        _view->show(Login);
    });

    connect(_verificationWidget, &VerificationWidget::done, [=] {
        _view->show(RegistrationSucceed);
        QTimer::singleShot(200, _registrationSucceedWidget,
                           &RegistrationSucceedWidget::start);
    });

    connect(_registrationSucceedWidget, &RegistrationSucceedWidget::done, [=] {
        _view->show(Login);
    });
}

void WelcomeWindow::showLogin()
{
    _view->show(Login);
}

void WelcomeWindow::showProjects()
{
//    _projectsWidget->refreshProjectList(); //FIXME
    _view->show(Projects);
    emit lazy();
}
