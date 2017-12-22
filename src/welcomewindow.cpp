#include <welcomewindow.h>
#include <projectswidget.h>
#include <loginwidget.h>
#include <view.h>
#include <fit.h>

WelcomeWindow::WelcomeWindow(QWidget* parent) : QWidget(parent)
{
    _loginWidget = new LoginWidget;
    _projectsWidget = new ProjectsWidget;

    connect(_loginWidget, SIGNAL(done()), SLOT(showProjects()));
    connect(_loginWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));
    connect(_projectsWidget, SIGNAL(done()), SIGNAL(done()));
    connect(_projectsWidget, SIGNAL(busy(QString)), SIGNAL(busy(QString)));

    _view = new View(this);
    _view->add(Projects, _projectsWidget);
    _view->add(Login, _loginWidget);
    _view->show(Login);
}

void WelcomeWindow::resizeEvent(QResizeEvent* event)
{
    _view->setGeometry(rect());
    QWidget::resizeEvent(event);
}

QSize WelcomeWindow::sizeHint() const
{
    return fit::fx(QSizeF{900, 600}).toSize();
}

void WelcomeWindow::showProjects()
{
    _projectsWidget->refreshProjectList();
    _view->show(Projects);
    emit comfy();
}
