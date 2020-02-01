#include <buildspane.h>
#include <view.h>
#include <platformswidget.h>
#include <androidplatformwidget.h>
#include <moduleselectionwidget.h>

BuildsPane::BuildsPane(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(QStringLiteral(APP_NAME) + QStringLiteral(" (Beta)"));

    _modulesWidget = new ModuleSelectionWidget;
    _androidWidget = new AndroidPlatformWidget;
    _platformsWidget = new PlatformsWidget;
//   FIXME _downloadWidget = new DownloadWidget;

    connect(_modulesWidget, &ModuleSelectionWidget::backClicked, this, &BuildsPane::showPlatforms);
    connect(_modulesWidget, &ModuleSelectionWidget::done, this, &BuildsPane::handleModuleSelection);
//    connect(_androidWidget, &AndroidPlatformWidget::backClicked, this, &BuildsPane::showModules);
    connect(_platformsWidget, &PlatformsWidget::platformSelected, this, &BuildsPane::handlePlatformSelection);
//    connect(_androidWidget, &AndroidPlatformWidget::downloadBuild, this, &BuildsPane::handleDownload);
//    connect(_downloadWidget, &DownloadWidget::done, this, &BuildsPane::handleModuleSelection);

    _view = new View(this);
    _view->add(Platforms, _platformsWidget);
    _view->add(Modules, _modulesWidget);
    _view->add(Android, _androidWidget);
//    _view->add(Download, _downloadWidget);
    _view->show(Android);

}

QSize BuildsPane::sizeHint() const
{
    return {670, 640};
}

QSize BuildsPane::minimumSizeHint() const
{
    return {0, 0};
}
