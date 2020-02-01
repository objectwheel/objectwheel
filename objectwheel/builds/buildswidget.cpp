#include <buildswidget.h>
#include <view.h>
#include <platformswidget.h>
#include <androidplatformwidget.h>
#include <moduleselectionwidget.h>

BuildsWidget::BuildsWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(QStringLiteral(APP_NAME) + QStringLiteral(" (Beta)"));

    _modulesWidget = new ModuleSelectionWidget;
    _androidWidget = new AndroidPlatformWidget;
    _platformsWidget = new PlatformsWidget;
//   FIXME _downloadWidget = new DownloadWidget;

    connect(_modulesWidget, &ModuleSelectionWidget::backClicked, this, &BuildsWidget::showPlatforms);
    connect(_modulesWidget, &ModuleSelectionWidget::done, this, &BuildsWidget::handleModuleSelection);
//    connect(_androidWidget, &AndroidPlatformWidget::backClicked, this, &BuildsWidget::showModules);
    connect(_platformsWidget, &PlatformsWidget::platformSelected, this, &BuildsWidget::handlePlatformSelection);
//    connect(_androidWidget, &AndroidPlatformWidget::downloadBuild, this, &BuildsWidget::handleDownload);
//    connect(_downloadWidget, &DownloadWidget::done, this, &BuildsWidget::handleModuleSelection);

    _view = new View(this);
    _view->add(Platforms, _platformsWidget);
    _view->add(Modules, _modulesWidget);
    _view->add(Android, _androidWidget);
//    _view->add(Download, _downloadWidget);
    _view->show(Android);

}

QSize BuildsWidget::sizeHint() const
{
    return QSize(670, 640);
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
//            _androidWidget->setTarget("Android Armeabi v7a");
            _view->show(Android);
            break;

        case OTargets::android_x86:
//            _androidWidget->setTarget("Android x86");
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
//    _downloadWidget->download(_target);
}
