#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class View;
class LoginWidget;
class ProjectsWidget;
class ProjectTemplatesWidget;
class ProjectDetailsWidget;
class RegistrationWidget;
class VerificationWidget;
class SucceedWidget;
class ForgetWidget;
class ResetWidget;

class WelcomeWindow : public QWidget
{
    Q_OBJECT

    friend class WindowManager;

    enum Screens {
        Login,
        Robot,
        Registration,
        Verification,
        Forget,
        Reset,
        Projects,
        ProjectTemplates,
        ProjectDetails,
        Succeed
    };

public:
    explicit WelcomeWindow(QWidget* parent = nullptr);

protected:
    QSize sizeHint() const override;

signals:
    void done();

private:
    View* m_view;
    LoginWidget* m_loginWidget;
    ProjectsWidget* m_projectsWidget;
    ProjectTemplatesWidget* m_projectTemplatesWidget;
    ProjectDetailsWidget* m_projectDetailsWidget;
    RegistrationWidget* m_registrationWidget;
    VerificationWidget* m_verificationWidget;
    SucceedWidget* m_succeedWidget;
    ForgetWidget* m_forgetWidget;
    ResetWidget* m_resetWidget;
};

#endif // WELCOMEWINDOW_H