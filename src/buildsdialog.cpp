#include <buildsdialog.h>

enum {
    Platforms
};

BuildsDialog::BuildsDialog(QWidget *parent)
    : QDialog(parent)
    , _view(this)
{
    _view.add(Platforms, &_platformsWidget);
    _view.show(Platforms);
}

void BuildsDialog::resizeEvent(QResizeEvent* event)
{
    _view.setGeometry(rect());
    QDialog::resizeEvent(event);
}
