#ifndef WELCOMEWINDOW_H
#define WELCOMEWINDOW_H

#include <QWidget>

class View;
class LoginWidget;
class ProjectsWidget;

class WelcomeWindow : public QWidget
{
        Q_OBJECT

    public:
        enum Screens { Login, Projects };

    public:
        explicit WelcomeWindow(QWidget* parent = nullptr);

    public slots:
        void showLogin();

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private slots:
        void showProjects();

    signals:
        void done();
        void lazy();
        void busy(const QString& text);

    private:
        View* _view;
        LoginWidget* _loginWidget;
        ProjectsWidget* _projectsWidget;
};

#endif // WELCOMEWINDOW_H