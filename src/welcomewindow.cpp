#include <welcomewindow.h>
#include <windowmanager.h>
#include <fit.h>
#include <view.h>
#include <loginwidget.h>
#include <robotwidget.h>
#include <projectswidget.h>
#include <projectdetailswidget.h>
#include <registrationwidget.h>
#include <verificationwidget.h>
#include <succeedwidget.h>
#include <forgetwidget.h>
#include <resetwidget.h>

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
    _projectDetailsWidget = new ProjectDetailsWidget;
    _verificationWidget = new VerificationWidget;
    _succeedWidget = new SucceedWidget;
    _forgetWidget = new ForgetWidget;
    _resetWidget = new ResetWidget;

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
    _view->add(ProjectDetails, _projectDetailsWidget);
    _view->add(Succeed, _succeedWidget);
    _view->add(Forget, _forgetWidget);
    _view->add(Reset, _resetWidget);
    _view->show(Login);

    connect(_forgetWidget, &ForgetWidget::back, [=] {
        _view->show(Login, View::LeftToRight);
    });

    connect(_forgetWidget, SIGNAL(done(QString)),
      _resetWidget, SLOT(setEmail(QString)));

    connect(_forgetWidget, &ForgetWidget::done, [=] {
        _view->show(Reset, View::RightToLeft);
    });

    connect(_resetWidget, &ResetWidget::cancel, [=] {
        _view->show(Login, View::LeftToRight);
    });

    connect(_resetWidget, &ResetWidget::done, [=] {
        _view->show(Succeed, View::RightToLeft);
        _succeedWidget->start();
        _succeedWidget->update(
          tr("Succeed."),
          tr("Your password has been successfully changed.\n"
          "You can continue by logging into the application with your new password."));
    });

    connect(_loginWidget, &LoginWidget::signup, [=] {
        _robotWidget->load();
        _view->show(Robot, View::RightToLeft);
    });

    connect(_loginWidget, &LoginWidget::forget, [=] {
        _view->show(Forget, View::RightToLeft);
    });

    connect(_loginWidget, &LoginWidget::about, [=] {
        WindowManager::instance()->show(WindowManager::About);
    });

    connect(_loginWidget, SIGNAL(busy(QString)),
      SIGNAL(busy(QString)));

    connect(_loginWidget, &LoginWidget::done, [=] {
        _projectsWidget->refreshProjectList();
        _view->show(Projects, View::RightToLeft);
        emit lazy();
    });

    connect(_projectsWidget, SIGNAL(busy(QString)),
      SIGNAL(busy(QString)));
    connect(_projectsWidget, SIGNAL(done()),
      SIGNAL(lazy()));
    connect(_projectsWidget, SIGNAL(done()),
      SIGNAL(done()));
    connect(_projectsWidget, SIGNAL(newProject(QString)),
      _projectDetailsWidget, SLOT(onNewProject(QString)));
    connect(_projectsWidget, SIGNAL(editProject(QString)),
      _projectDetailsWidget, SLOT(onEditProject(QString)));
    connect(_projectsWidget, &ProjectsWidget::newProject, [=] {
        _view->show(ProjectDetails, View::RightToLeft);
    });
    connect(_projectsWidget, &ProjectsWidget::editProject, [=] {
        _view->show(ProjectDetails, View::RightToLeft);
    });

    connect(_projectDetailsWidget, &ProjectDetailsWidget::done, [=] {
        _projectsWidget->refreshProjectList();
        _view->show(Projects, View::LeftToRight);
    });

    connect(_robotWidget, SIGNAL(done(QString)),
      _registrationWidget, SLOT(updateResponse(QString)));

    connect(_robotWidget, &RobotWidget::back, [=] {
        _view->show(Login, View::LeftToRight);
    });

    connect(_robotWidget, &RobotWidget::done, [=] {
        _view->show(Registration, View::RightToLeft);
    });

    connect(_registrationWidget, &RegistrationWidget::back, [=] {
        _view->show(Login, View::LeftToRight);
    });

    connect(_registrationWidget, SIGNAL(done(QString)),
      _verificationWidget, SLOT(setEmail(QString)));

    connect(_registrationWidget, &RegistrationWidget::done, [=] {
        _view->show(Verification, View::RightToLeft);
        _robotWidget->reset();
    });

    connect(_verificationWidget, &VerificationWidget::cancel, [=] {
        _view->show(Login, View::LeftToRight);
    });

    connect(_verificationWidget, &VerificationWidget::done, [=] {
        _view->show(Succeed, View::RightToLeft);
        _succeedWidget->start();
        _succeedWidget->update(
          tr("Thank you for registering."),
          tr("Your registration is completed. Thank you for choosing us.\n"
          "You can continue by logging into the application."));
    });

    connect(_succeedWidget, &SucceedWidget::done, [=] {
        _view->show(Login, View::LeftToRight);
    });
}

