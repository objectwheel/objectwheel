#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class View;
class QVBoxLayout;
class LoginWidget;
class RobotWidget;
class ProjectsWidget;
class ProjectDetailsWidget;
class RegistrationWidget;
class VerificationWidget;
class SucceedWidget;
class ForgetWidget;
class ResetWidget;

class WelcomeWindow : public QWidget
{
        Q_OBJECT

    public:
        enum Screens {
            Login,
            Robot,
            Registration,
            Verification,
            Forget,
            Reset,
            Projects,
            NewProject,
            Succeed
        };

    public:
        explicit WelcomeWindow(QWidget* parent = nullptr);

    signals:
        void done();
        void lazy();
        void busy(const QString& text);

    private:
        View* _view;
        QVBoxLayout* _layout;
        LoginWidget* _loginWidget;
        RobotWidget* _robotWidget;
        ProjectsWidget* _projectsWidget;
        ProjectDetailsWidget* _projectDetailsWidget;
        RegistrationWidget* _registrationWidget;
        VerificationWidget* _verificationWidget;
        SucceedWidget* _succeedWidget;
        ForgetWidget* _forgetWidget;
        ResetWidget* _resetWidget;
};

#endif // WELCOMEWINDOW_H