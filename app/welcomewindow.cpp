#include <welcomewindow.h>
#include <windowmanager.h>
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
#include <projecttemplateswidget.h>
#include <aboutwindow.h>

#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#f0f0f0");
    setPalette(p);
    setWindowTitle(APP_NAME);

    m_view = new View(this);
    m_loginWidget = new LoginWidget;
    m_robotWidget = new RobotWidget;
    m_registrationWidget = new RegistrationWidget;
    m_projectsWidget = new ProjectsWidget;
    m_projectTemplatesWidget = new ProjectTemplatesWidget;
    m_projectDetailsWidget = new ProjectDetailsWidget;
    m_verificationWidget = new VerificationWidget;
    m_succeedWidget = new SucceedWidget;
    m_forgetWidget = new ForgetWidget;
    m_resetWidget = new ResetWidget;

    m_view->add(Login, m_loginWidget);
    m_view->add(Robot, m_robotWidget);
    m_view->add(Registration, m_registrationWidget);
    m_view->add(Verification, m_verificationWidget);
    m_view->add(Projects, m_projectsWidget);
    m_view->add(ProjectTemplates, m_projectTemplatesWidget);
    m_view->add(ProjectDetails, m_projectDetailsWidget);
    m_view->add(Succeed, m_succeedWidget);
    m_view->add(Forget, m_forgetWidget);
    m_view->add(Reset, m_resetWidget);
    m_view->show(Login);

    /**** ForgetWidget settings ****/
    connect(m_forgetWidget, SIGNAL(done(QString)), m_resetWidget, SLOT(setEmail(QString)));
    connect(m_forgetWidget, &ForgetWidget::back, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
    connect(m_forgetWidget, &ForgetWidget::done, [=]
    {
        m_view->show(Reset, View::RightToLeft);
    });

    /**** ResetWidget settings ****/
    connect(m_resetWidget, &ResetWidget::cancel, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
    connect(m_resetWidget, &ResetWidget::done, [=] {
        m_view->show(Succeed, View::RightToLeft);
        m_succeedWidget->start();
        m_succeedWidget->update(
          tr("Succeed."),
          tr("Your password has been successfully changed.\n"
          "You can continue by logging into the application with your new password."));
    });

    /**** LoginWidget settings ****/
    connect(m_loginWidget, &LoginWidget::signup, [=]
    {
        m_robotWidget->load();
        m_view->show(Robot, View::RightToLeft);
    });
    connect(m_loginWidget, &LoginWidget::forget, [=]
    {
        m_view->show(Forget, View::RightToLeft);
    });
    connect(m_loginWidget, &LoginWidget::about, [=]
    {
        WindowManager::aboutWindow()->show();
    });
    connect(m_loginWidget, &LoginWidget::done, [=]
    {
        m_projectsWidget->refreshProjectList();
        m_view->show(Projects, View::RightToLeft);
    });

    /**** ProjectsWidget settings ****/
    connect(m_projectsWidget, SIGNAL(done()), SIGNAL(done()));
    connect(m_projectsWidget, SIGNAL(newProject(QString)), m_projectTemplatesWidget, SLOT(onNewProject(QString)));
    connect(m_projectsWidget, SIGNAL(editProject(QString)), m_projectDetailsWidget, SLOT(onEditProject(QString)));
    connect(m_projectsWidget, &ProjectsWidget::newProject, [=]
    {
        m_view->show(ProjectTemplates, View::RightToLeft);
    });
    connect(m_projectsWidget, &ProjectsWidget::editProject, [=]
    {
        m_view->show(ProjectDetails, View::RightToLeft);
    });

    /**** ProjectTemplatesWidget settings ****/
    connect(m_projectTemplatesWidget, SIGNAL(newProject(QString, int)), m_projectDetailsWidget, SLOT(onNewProject(QString, int)));
    connect(m_projectTemplatesWidget, &ProjectTemplatesWidget::back, [=]
    {
        m_projectsWidget->refreshProjectList();
        m_view->show(Projects, View::LeftToRight);
    });
    connect(m_projectTemplatesWidget, &ProjectTemplatesWidget::newProject, [=]
    {
        m_view->show(ProjectDetails, View::RightToLeft);
    });

    /**** ProjectDetailsWidget settings ****/
    connect(m_projectDetailsWidget, &ProjectDetailsWidget::back, [=]
    {
        m_view->show(ProjectTemplates, View::LeftToRight);
    });
    connect(m_projectDetailsWidget, &ProjectDetailsWidget::done, [=]
    {
        m_projectsWidget->refreshProjectList();
        m_view->show(Projects, View::LeftToRight);
    });

    /**** RobotWidget settings ****/
    connect(m_robotWidget, SIGNAL(done(QString)), m_registrationWidget, SLOT(updateResponse(QString)));
    connect(m_robotWidget, &RobotWidget::back, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
    connect(m_robotWidget, &RobotWidget::done, [=]
    {
        m_view->show(Registration, View::RightToLeft);
    });

    /**** RegistrationWidget settings ****/
    connect(m_registrationWidget, &RegistrationWidget::back, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
    connect(m_registrationWidget, SIGNAL(done(QString)), m_verificationWidget, SLOT(setEmail(QString)));
    connect(m_registrationWidget, &RegistrationWidget::done, [=]
    {
        m_view->show(Verification, View::RightToLeft);
        m_robotWidget->reset();
    });

    /**** VerificationWidget settings ****/
    connect(m_verificationWidget, &VerificationWidget::cancel, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
    connect(m_verificationWidget, &VerificationWidget::done, [=]
    {
        m_view->show(Succeed, View::RightToLeft);
        m_succeedWidget->start();
        m_succeedWidget->update(
          tr("Thank you for registering."),
          tr("Your registration is completed. Thank you for choosing us.\n"
          "You can continue by logging into the application."));
    });

    /**** SucceedWidget settings ****/
    connect(m_succeedWidget, &SucceedWidget::done, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
}

QSize WelcomeWindow::sizeHint() const
{
    return {980, 560};
}

