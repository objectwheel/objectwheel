#include <buildsdialog.h>

enum {
    Platforms,
    Android,
};

BuildsDialog::BuildsDialog(QWidget *parent)
    : QDialog(parent)
    , _view(this)
{
    _view.add(Platforms, &_platformsWidget);
    _view.add(Android, &_androidWidget);
    _view.show(Platforms);

    connect(&_androidWidget, SIGNAL(backClicked()),
      SLOT(showPlatforms()));
    connect(&_platformsWidget, SIGNAL(platformSelected(Targets)),
      SLOT(handlePlatformSelection(Targets)));
}

void BuildsDialog::resizeEvent(QResizeEvent* event)
{
    _view.setGeometry(rect());
    QDialog::resizeEvent(event);
}

void BuildsDialog::showPlatforms()
{
    _view.show(Platforms);
}

void BuildsDialog::handlePlatformSelection(Targets platform)
{
    switch (platform) {
        case android_armeabi_v7a:
            _androidWidget.setTarget("Android Armeabi v7a");
            _view.show(Android);
            break;

        case android_x86:
            _androidWidget.setTarget("Android x86");
            _view.show(Android);
            break;
        default:
            break;
    }
}
