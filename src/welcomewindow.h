#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class View;
class QVBoxLayout;
class LoginWidget;
class RobotWidget;
class ProjectsWidget;
class NewProjectWidget;
class RegistrationWidget;
class VerificationWidget;
class RegistrationSucceedWidget;
class ForgetWidget;

class WelcomeWindow : public QWidget
{
        Q_OBJECT

    public:
        enum Screens {
            Login,
            Robot,
            Forget,
            Registration,
            Verification,
            Projects,
            NewProject,
            RegistrationSucceed
        };

    public:
        explicit WelcomeWindow(QWidget* parent = nullptr);

    public slots:
        void showLogin();

    private slots:
        void showProjects();

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
        NewProjectWidget* _newProjectWidget;
        RegistrationWidget* _registrationWidget;
        VerificationWidget* _verificationWidget;
        RegistrationSucceedWidget* _registrationSucceedWidget;
        ForgetWidget* _forgetWidget;
};

#endif // WELCOMEWINDOW_H