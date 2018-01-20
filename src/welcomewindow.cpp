#include <fit.h>
#include <view.h>
#include <global.h>
#include <welcomewindow.h>
#include <loginwidget.h>
#include <projectswidget.h>
#include <newprojectwidget.h>
#include <registrationwidget.h>
#include <verificationwidget.h>
#include <registrationsucceedwidget.h>
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
    _legalLabel = new QLabel;
    _loginWidget = new LoginWidget;
    _registrationWidget = new RegistrationWidget;
    _projectsWidget = new ProjectsWidget;
    _newProjectWidget = new NewProjectWidget;
    _verificationWidget = new VerificationWidget;
    _registrationSucceedWidget = new RegistrationSucceedWidget;

    _layout->setSpacing(fit::fx(12));
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSizeConstraint(QLayout::SetMaximumSize);

    _layout->addWidget(_view);
    _layout->addWidget(_legalLabel);

    connect(_projectsWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_loginWidget, SIGNAL(done()), SLOT(showProjects()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(lazy()));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(done()));
    connect(_registrationWidget, SIGNAL(done(QString)),
      _verificationWidget, SLOT(setEmail(QString)));

    connect(_loginWidget, &LoginWidget::signup, [=] {
        _view->show(Registration);
    });

    connect(_registrationWidget, &RegistrationWidget::cancel, [=] {
        _view->show(Login);
    });

    connect(_registrationWidget, &RegistrationWidget::done, [=] {
        _view->show(Verification);
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

    _view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _view->add(Login, _loginWidget);
    _view->add(Registration, _registrationWidget);
    _view->add(Verification, _verificationWidget);
    _view->add(Projects, _projectsWidget);
    _view->add(NewProject, _newProjectWidget);
    _view->add(RegistrationSucceed, _registrationSucceedWidget);
    _view->show(Login);

    _legalLabel->setText(TEXT_LEGAL);
    _legalLabel->setStyleSheet("color:#2E3A41;");
    _legalLabel->setAlignment(Qt::AlignHCenter);
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
