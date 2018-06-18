#include <buildswidget.h>
#include <view.h>
#include <platformswidget.h>
#include <androidwidget.h>
#include <moduleselectionwidget.h>
#include <downloadwidget.h>

BuildsWidget::BuildsWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(APP_NAME);

    _modulesWidget = new ModuleSelectionWidget;
    _androidWidget = new AndroidWidget;
    _platformsWidget = new PlatformsWidget;
    _downloadWidget = new DownloadWidget;

    connect(_modulesWidget, SIGNAL(backClicked()), SLOT(showPlatforms()));
    connect(_modulesWidget, SIGNAL(done()), SLOT(handleModuleSelection()));
    connect(_androidWidget, SIGNAL(backClicked()), SLOT(showModules()));
    connect(_platformsWidget, SIGNAL(platformSelected(OTargets::Targets)), SLOT(handlePlatformSelection(OTargets::Targets)));
    connect(_androidWidget, SIGNAL(downloadBuild()), SLOT(handleDownload()));
    connect(_downloadWidget, SIGNAL(done()), SLOT(handleModuleSelection()));

    _view = new View(this);
    _view->add(Platforms, _platformsWidget);
    _view->add(Modules, _modulesWidget);
    _view->add(Android, _androidWidget);
    _view->add(Download, _downloadWidget);
    _view->show(Platforms);
}

void BuildsWidget::sweep()
{
    //TODO
}

QSize BuildsWidget::sizeHint() const
{
    return QSize(680, 680);
}

void BuildsWidget::showModules()
{
    _view->show(Modules);
}

void BuildsWidget::showPlatforms()
{
    _view->show(Platforms);
}

void BuildsWidget::handleModuleSelection()
{
    switch (_target) {
        case OTargets::android_armeabi_v7a:
            _androidWidget->setTarget("Android Armeabi v7a");
            _view->show(Android);
            break;

        case OTargets::android_x86:
            _androidWidget->setTarget("Android x86");
            _view->show(Android);
            break;
        default:
            break;
    }
}

void BuildsWidget::handlePlatformSelection(OTargets::Targets platform)
{
    _target = platform;
    _view->show(Modules);
}

void BuildsWidget::handleDownload()
{
    _view->show(Download);
    _downloadWidget->download(_target);
}
