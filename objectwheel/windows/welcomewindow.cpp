#include <welcomewindow.h>
#include <windowmanager.h>
#include <view.h>
#include <loginwidget.h>
#include <projectswidget.h>
#include <projectdetailswidget.h>
#include <registrationwidget.h>
#include <verificationwidget.h>
#include <succeedwidget.h>
#include <forgetwidget.h>
#include <resetwidget.h>
#include <projecttemplateswidget.h>
#include <aboutwindow.h>
#include <utilityfunctions.h>

#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QCloseEvent>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle(APP_NAME + QStringLiteral(" (Beta)"));

    m_view = new View(this);
    m_loginWidget = new LoginWidget;
    m_registrationWidget = new RegistrationWidget;
    m_projectsWidget = new ProjectsWidget;
    m_projectTemplatesWidget = new ProjectTemplatesWidget;
    m_projectDetailsWidget = new ProjectDetailsWidget;
    m_verificationWidget = new VerificationWidget;
    m_succeedWidget = new SucceedWidget;
    m_forgetWidget = new ForgetWidget;
    m_resetWidget = new ResetWidget;

    m_view->add(Login, m_loginWidget);
    m_view->add(Registration, m_registrationWidget);
    m_view->add(Verification, m_verificationWidget);
    m_view->add(Projects, m_projectsWidget);
    m_view->add(ProjectTemplates, m_projectTemplatesWidget);
    m_view->add(ProjectDetails, m_projectDetailsWidget);
    m_view->add(Succeed, m_succeedWidget);
    m_view->add(Forget, m_forgetWidget);
    m_view->add(Reset, m_resetWidget);
    m_view->show(Login);

    resize(sizeHint());
    move(UtilityFunctions::centerPos(size()));

    /**** ForgetWidget settings ****/
    connect(m_forgetWidget, &ForgetWidget::done, m_resetWidget, &ResetWidget::setEmail);
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
        m_view->show(Registration, View::RightToLeft);
    });
    connect(m_loginWidget, &LoginWidget::resetPassword, [=]
    {
        m_view->show(Forget, View::RightToLeft);
    });
    connect(m_loginWidget, &LoginWidget::about, [=]
    {
        WindowManager::aboutWindow()->show();
        WindowManager::aboutWindow()->activateWindow();
    });
    connect(m_loginWidget, &LoginWidget::done, [=]
    {
        m_projectsWidget->refreshProjectList();
        m_view->show(Projects, View::RightToLeft);
    });
    /**** ProjectsWidget settings ****/
    connect(m_projectsWidget, &ProjectsWidget::done,
            this, &WelcomeWindow::done);
    connect(m_projectsWidget, &ProjectsWidget::newProject,
            m_projectTemplatesWidget, &ProjectTemplatesWidget::onNewProject);
    connect(m_projectsWidget, &ProjectsWidget::editProject,
            m_projectDetailsWidget, &ProjectDetailsWidget::onEditProject);
    connect(m_projectsWidget, &ProjectsWidget::newProject, [=]
    {
        m_view->show(ProjectTemplates, View::RightToLeft);
    });
    connect(m_projectsWidget, &ProjectsWidget::editProject, [=]
    {
        m_view->show(ProjectDetails, View::RightToLeft);
    });
    /**** ProjectTemplatesWidget settings ****/
    connect(m_projectTemplatesWidget, &ProjectTemplatesWidget::newProject,
            m_projectDetailsWidget, &ProjectDetailsWidget::onNewProject);
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
        m_projectsWidget->refreshProjectList(true);
        m_view->show(Projects, View::LeftToRight);
    });
    /**** RegistrationWidget settings ****/
    connect(m_registrationWidget, &RegistrationWidget::back, [=]
    {
        m_view->show(Login, View::LeftToRight);
    });
    connect(m_registrationWidget, &RegistrationWidget::done,
            m_verificationWidget, &VerificationWidget::setEmail);
    connect(m_registrationWidget, &RegistrationWidget::done, [=]
    {
        m_view->show(Verification, View::RightToLeft);
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
