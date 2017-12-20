#include <buildsdialog.h>

enum {
    Platforms,
    Modules,
    Android,
    Download
};

BuildsDialog::BuildsDialog(QWidget *parent)
    : QDialog(parent)
    , _view(this)
{
    _view.add(Platforms, &_platformsWidget);
    _view.add(Modules, &_modulesWidget);
    _view.add(Android, &_androidWidget);
    _view.add(Download, &_downloadWidget);
    _view.show(Platforms);

    connect(&_modulesWidget, SIGNAL(backClicked()),
      SLOT(showPlatforms()));
    connect(&_modulesWidget, SIGNAL(done()),
      SLOT(handleModuleSelection()));
    connect(&_androidWidget, SIGNAL(backClicked()),
      SLOT(showModules()));
    connect(&_platformsWidget, SIGNAL(platformSelected(OTargets::Targets)),
      SLOT(handlePlatformSelection(OTargets::Targets)));
    connect(&_androidWidget, SIGNAL(downloadBuild()),
      SLOT(handleDownload()));
    connect(&_downloadWidget, SIGNAL(done()),
      SLOT(handleModuleSelection()));
}

void BuildsDialog::resizeEvent(QResizeEvent* event)
{
    _view.setGeometry(rect());
    QDialog::resizeEvent(event);
}

void BuildsDialog::showModules()
{
    _view.show(Modules);
}

void BuildsDialog::showPlatforms()
{
    _view.show(Platforms);
}

void BuildsDialog::handleModuleSelection()
{
    switch (_target) {
        case OTargets::android_armeabi_v7a:
            _androidWidget.setTarget("Android Armeabi v7a");
            _view.show(Android);
            break;

        case OTargets::android_x86:
            _androidWidget.setTarget("Android x86");
            _view.show(Android);
            break;
        default:
            break;
    }
}

void BuildsDialog::handlePlatformSelection(OTargets::Targets platform)
{
    _target = platform;
    _view.show(Modules);
}

void BuildsDialog::handleDownload()
{
    _view.show(Download);
    _downloadWidget.download(_target);
}